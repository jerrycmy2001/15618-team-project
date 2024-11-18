#include <algorithm>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <vector>

#include <cuda.h>
#include <cuda_runtime.h>
#include <driver_functions.h>

#include "cudaRenderer.h"
#include "image.h"
#include "noise.h"
#include "sceneLoader.h"
#include "util.h"

#define BLOCK_WIDTH 32
#define THREADS_PER_BLOCK (BLOCK_WIDTH * BLOCK_WIDTH)
#define BATCH_SIZE (THREADS_PER_BLOCK * 2)

#define DEBUG
#ifdef DEBUG
#define cudaCheckError(ans) cudaAssert((ans), __FILE__, __LINE__);
inline void cudaAssert(cudaError_t code, const char *file, int line,
                       bool abort = true) {
  if (code != cudaSuccess) {
    fprintf(stderr, "CUDA Error: %s at %s:%d\n", cudaGetErrorString(code), file,
            line);
    if (abort)
      exit(code);
  }
}
#else
#define cudaCheckError(ans) ans
#endif

////////////////////////////////////////////////////////////////////////////////////////
// All cuda kernels here
///////////////////////////////////////////////////////////////////////////////////////

// This stores the global constants
struct GlobalConstants {

  SceneName sceneName;

  int numPolygons;
  float *vertices;
  float *colors;

  int imageWidth;
  int imageHeight;
  float *imageData;
};

// Global variable that is in scope, but read-only, for all cuda
// kernels.  The __constant__ modifier designates this variable will
// be stored in special "constant" memory on the GPU. (we didn't talk
// about this type of memory in class, but constant memory is a fast
// place to put read-only variables).
__constant__ GlobalConstants cuConstRendererParams;

// Read-only lookup tables used to quickly compute noise (needed by
// advanceAnimation for the snowflake scene)
__constant__ int cuConstNoiseYPermutationTable[256];
__constant__ int cuConstNoiseXPermutationTable[256];
__constant__ float cuConstNoise1DValueTable[256];

// Color ramp table needed for the color ramp lookup shader
#define COLOR_MAP_SIZE 5
__constant__ float cuConstColorRamp[COLOR_MAP_SIZE][3];

// kernelClearImage --  (CUDA device code)
//
// Clear the image, setting all pixels to the specified color rgba
__global__ void kernelClearImage(float r, float g, float b, float a) {

  int imageX = blockIdx.x * blockDim.x + threadIdx.x;
  int imageY = blockIdx.y * blockDim.y + threadIdx.y;

  int width = cuConstRendererParams.imageWidth;
  int height = cuConstRendererParams.imageHeight;

  if (imageX >= width || imageY >= height)
    return;

  int offset = 4 * (imageY * width + imageX);
  float4 value = make_float4(r, g, b, a);

  // Write to global memory: As an optimization, this code uses a float4
  // store, which results in more efficient code than if it were coded as
  // four separate float stores.
  *(float4 *)(&cuConstRendererParams.imageData[offset]) = value;
}

// kernelAdvanceCamera -- (CUDA device code)
//
// Update the camera position for the camera path animation
__global__ void kernelAdvanceCamera() {}

// scan kernel that is only valid for N <= 2 * THREADS_PER_BLOCK
// use template to unroll the for loop in compile time
template <int N> __device__ void scanKernel(short *data) {
  int threadId = threadIdx.y * blockDim.x + threadIdx.x;
// upsweep phase.
#pragma unroll
  for (int twod = 1; twod < N; twod *= 2) {
    int twod1 = twod * 2;
    for (int i = threadId * twod1; i < N; i += THREADS_PER_BLOCK * twod1) {
      data[i + twod1 - 1] += data[i + twod - 1];
    }
    __syncthreads();
  }
  if (threadId == 0) {
    data[N - 1] = 0;
  }
  __syncthreads();

// downsweep phase.
#pragma unroll
  for (int twod = N / 2; twod >= 1; twod /= 2) {
    int twod1 = twod * 2;
    for (int i = threadId * twod1; i < N; i += THREADS_PER_BLOCK * twod1) {
      int t = data[i + twod - 1];
      data[i + twod - 1] = data[i + twod1 - 1];
      // change twod1 below to twod to reverse prefix sum.
      data[i + twod1 - 1] += t;
    }
    __syncthreads();
  }
}

// scan kernel that is only valid for N <= 2 * THREADS_PER_BLOCK
__device__ void scanKernelVariable(short *data, int N) {
  int threadId = threadIdx.y * blockDim.x + threadIdx.x;
  // upsweep phase.
  for (int twod = 1; twod < N; twod *= 2) {
    int twod1 = twod * 2;
    for (int i = threadId * twod1; i < N; i += THREADS_PER_BLOCK * twod1) {
      data[i + twod1 - 1] += data[i + twod - 1];
    }
    __syncthreads();
  }
  if (threadId == 0) {
    data[N - 1] = 0;
  }
  __syncthreads();

  // downsweep phase.
  for (int twod = N / 2; twod >= 1; twod /= 2) {
    int twod1 = twod * 2;
    for (int i = threadId * twod1; i < N; i += THREADS_PER_BLOCK * twod1) {
      int t = data[i + twod - 1];
      data[i + twod - 1] = data[i + twod1 - 1];
      // change twod1 below to twod to reverse prefix sum.
      data[i + twod1 - 1] += t;
    }
    __syncthreads();
  }
}

// write index in input to output
__device__ void writeIndex(short *input, short *output, int N) {
  int threadId = threadIdx.y * blockDim.x + threadIdx.x;

  for (int offset = 0; offset < N; offset += THREADS_PER_BLOCK) {
    if (offset + threadId < N && offset + threadId > 0) {
      int curr = input[offset + threadId];
      if (curr > input[offset + threadId - 1]) {
        output[curr - 1] = offset + threadId - 1;
      }
    }

    __syncthreads();
  }
}

/* Helper function to round up to a power of 2.
 */
__device__ inline int nextPow2(int n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  return n;
}

// check if a circle intersects with box
__device__ __inline__ int circleInBox(float circleX, float circleY,
                                      float circleRadius, float boxL,
                                      float boxR, float boxT, float boxB) {

  // clamp circle center to box (finds the closest point on the box)
  float closestX =
      (circleX > boxL) ? ((circleX < boxR) ? circleX : boxR) : boxL;
  float closestY =
      (circleY > boxB) ? ((circleY < boxT) ? circleY : boxT) : boxB;

  // is circle radius less than the distance to the closest point on
  // the box?
  float distX = closestX - circleX;
  float distY = closestY - circleY;

  if (((distX * distX) + (distY * distY)) <= (circleRadius * circleRadius)) {
    return 1;
  } else {
    return 0;
  }
}

// render the image by pixels
__global__ void kernelRenderPixels() {}

////////////////////////////////////////////////////////////////////////////////////////

CudaRenderer::CudaRenderer() {
  image = NULL;

  cudaDeviceVertices = NULL;
  cudaDeviceEndIndices = NULL;
  cudaDeviceColors = NULL;
}

CudaRenderer::~CudaRenderer() {

  if (image) {
    delete image;
  }

  if (cudaDeviceVertices) {
    cudaFree(cudaDeviceVertices);
    cudaFree(cudaDeviceEndIndices);
    cudaFree(cudaDeviceColors);
  }
}

const Image *CudaRenderer::getImage() {

  // Need to copy contents of the rendered image from device memory
  // before we expose the Image object to the caller

  printf("Copying image data from device\n");

  cudaMemcpy(image->data, cudaDeviceImageData,
             sizeof(float) * 4 * image->width * image->height,
             cudaMemcpyDeviceToHost);

  return image;
}

void CudaRenderer::loadScene(SceneName name) {
  sceneName = name;
  ::loadScene(sceneName, image->width, image->height);
  scene->serialize(numTriangles, vertices, colors);
}

void CudaRenderer::setup() {

  int deviceCount = 0;
  bool isFastGPU = false;
  std::string name;
  cudaError_t err = cudaGetDeviceCount(&deviceCount);

  printf("---------------------------------------------------------\n");
  printf("Initializing CUDA for CudaRenderer\n");
  printf("Found %d CUDA devices\n", deviceCount);

  for (int i = 0; i < deviceCount; i++) {
    cudaDeviceProp deviceProps;
    cudaGetDeviceProperties(&deviceProps, i);
    name = deviceProps.name;
    if (name.compare("GeForce RTX 2080") == 0) {
      isFastGPU = true;
    }

    printf("Device %d: %s\n", i, deviceProps.name);
    printf("   SMs:        %d\n", deviceProps.multiProcessorCount);
    printf("   Global mem: %.0f MB\n",
           static_cast<float>(deviceProps.totalGlobalMem) / (1024 * 1024));
    printf("   CUDA Cap:   %d.%d\n", deviceProps.major, deviceProps.minor);
  }
  printf("---------------------------------------------------------\n");
  if (!isFastGPU) {
    printf("WARNING: "
           "You're not running on a fast GPU, please consider using "
           "NVIDIA RTX 2080.\n");
    printf("---------------------------------------------------------\n");
  }

  // By this time the scene should be loaded.  Now copy all the key
  // data structures into device memory so they are accessible to
  // CUDA kernels
  //
  // See the CUDA Programmer's Guide for descriptions of
  // cudaMalloc and cudaMemcpy

  cudaMalloc(&cudaDeviceVertices, sizeof(float) * 3 * numTriangles);
  cudaMalloc(&cudaDeviceColors, sizeof(float) * 4 * numTriangles);
  cudaMalloc(&cudaDeviceImageData,
             sizeof(float) * 4 * image->width * image->height);

  // cudaMemcpy(cudaDevicePosition, position, sizeof(float) * 3 *
  // numberOfCircles,
  //            cudaMemcpyHostToDevice);
  // cudaMemcpy(cudaDeviceVelocity, velocity, sizeof(float) * 3 *
  // numberOfCircles,
  //            cudaMemcpyHostToDevice);
  // cudaMemcpy(cudaDeviceColor, color, sizeof(float) * 3 * numberOfCircles,
  //            cudaMemcpyHostToDevice);
  // cudaMemcpy(cudaDeviceRadius, radius, sizeof(float) * numberOfCircles,
  //            cudaMemcpyHostToDevice);

  // Initialize parameters in constant memory.  We didn't talk about
  // constant memory in class, but the use of read-only constant
  // memory here is an optimization over just sticking these values
  // in device global memory.  NVIDIA GPUs have a few special tricks
  // for optimizing access to constant memory.  Using global memory
  // here would have worked just as well.  See the Programmer's
  // Guide for more information about constant memory.

  GlobalConstants params;
  params.sceneName = sceneName;
  params.numPolygons = numTriangles;
  params.vertices = cudaDeviceVertices;
  params.colors = cudaDeviceColors;
  params.imageWidth = image->width;
  params.imageHeight = image->height;
  params.imageData = cudaDeviceImageData;

  cudaMemcpyToSymbol(cuConstRendererParams, &params, sizeof(GlobalConstants));

  // Also need to copy over the noise lookup tables, so we can
  // implement noise on the GPU
  int *permX;
  int *permY;
  float *value1D;
  getNoiseTables(&permX, &permY, &value1D);
  cudaMemcpyToSymbol(cuConstNoiseXPermutationTable, permX, sizeof(int) * 256);
  cudaMemcpyToSymbol(cuConstNoiseYPermutationTable, permY, sizeof(int) * 256);
  cudaMemcpyToSymbol(cuConstNoise1DValueTable, value1D, sizeof(float) * 256);

  // Copy over the color table that's used by the shading
  // function for circles in the snowflake demo

  float lookupTable[COLOR_MAP_SIZE][3] = {
      {1.f, 1.f, 1.f}, {1.f, 1.f, 1.f},  {.8f, .9f, 1.f},
      {.8f, .9f, 1.f}, {.8f, 0.8f, 1.f},
  };

  cudaMemcpyToSymbol(cuConstColorRamp, lookupTable,
                     sizeof(float) * 3 * COLOR_MAP_SIZE);
}

// allocOutputImage --
//
// Allocate buffer the renderer will render into.  Check status of
// image first to avoid memory leak.
void CudaRenderer::allocOutputImage(int width, int height) {

  if (image)
    delete image;
  image = new Image(width, height);
}

// clearImage --
//
// Clear the renderer's target image.  The state of the image after
// the clear depends on the scene being rendered.
void CudaRenderer::clearImage() {

  // 256 threads per block is a healthy number
  dim3 blockDim(16, 16, 1);
  dim3 gridDim((image->width + blockDim.x - 1) / blockDim.x,
               (image->height + blockDim.y - 1) / blockDim.y);

  kernelClearImage<<<gridDim, blockDim>>>(1.f, 1.f, 1.f, 1.f);
  cudaDeviceSynchronize();
}

// advanceAnimation --
//
// Advance the simulation one time step.  Updates all circle positions
// and velocities
void CudaRenderer::advanceAnimation() {
  // 256 threads per block is a healthy number
  dim3 blockDim(1, 1);
  dim3 gridDim(1, 1);

  // only the snowflake scene has animation
  kernelAdvanceCamera<<<gridDim, blockDim>>>();
  cudaDeviceSynchronize();
}

void CudaRenderer::render() {
  // 256 threads per block is a healthy number
  //   dim3 blockDim(256, 1);
  //   dim3 gridDim((numberOfCircles + blockDim.x - 1) / blockDim.x);
  //   kernelRenderCircles<<<gridDim, blockDim>>>();
  //   cudaDeviceSynchronize();

  int imageWidth = image->width;
  int imageHeight = image->height;
  dim3 blockDim(BLOCK_WIDTH, BLOCK_WIDTH);
  dim3 gridDim(ceil((double)imageWidth / (double)BLOCK_WIDTH),
               ceil((double)imageHeight / (double)BLOCK_WIDTH));
  kernelRenderPixels<<<gridDim, blockDim>>>();
  cudaCheckError(cudaDeviceSynchronize())
}