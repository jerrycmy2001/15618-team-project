#include <algorithm>
#include <cmath>
#include <string>
#define _USE_MATH_DEFINES
#include <cfloat>
#include <math.h>
#include <omp.h>
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

  int numTriangles;
  float *vertices;
  float *colors;

  int imageWidth;
  int imageHeight;
  float *imageData;

  float combinedMatrix[4][4];
};

// Global variable that is in scope, but read-only, for all cuda
// kernels.  The __constant__ modifier designates this variable will
// be stored in special "constant" memory on the GPU. (we didn't talk
// about this type of memory in class, but constant memory is a fast
// place to put read-only variables).
// __constant__ GlobalConstants cuConstRendererParams;

// Color ramp table needed for the color ramp lookup shader
#define COLOR_MAP_SIZE 5
__constant__ float cuConstColorRamp[COLOR_MAP_SIZE][3];

// kernelClearImage --  (CUDA device code)
//
// Clear the image, setting all pixels to the specified color rgba
__global__ void kernelClearImage(float r, float g, float b, float a) {

  // int imageX = blockIdx.x * blockDim.x + threadIdx.x;
  // int imageY = blockIdx.y * blockDim.y + threadIdx.y;

  // int width = cuConstRendererParams.imageWidth;
  // int height = cuConstRendererParams.imageHeight;

  // if (imageX >= width || imageY >= height)
  //   return;

  // int offset = 4 * (imageY * width + imageX);
  // float4 value = make_float4(r, g, b, a);

  // // Write to global memory: As an optimization, this code uses a float4
  // // store, which results in more efficient code than if it were coded as
  // // four separate float stores.
  // *(float4 *)(&cuConstRendererParams.imageData[offset]) = value;
}

__global__ void kernelClearImage(float *image, int imageWidth, int imageHeight,
                                 float r, float g, float b, float a) {
  int imageX = blockIdx.x * blockDim.x + threadIdx.x;
  int imageY = blockIdx.y * blockDim.y + threadIdx.y;

  if (imageX >= imageWidth || imageY >= imageHeight)
    return;

  int offset = 4 * (imageY * imageWidth + imageX);
  float4 value = make_float4(r, g, b, a);

  // Write to global memory: As an optimization, this code uses a float4
  // store, which results in more efficient code than if it were coded as
  // four separate float stores.
  *(float4 *)(&image[offset]) = value;
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

static __device__ void transformVertex(float *vertex, float *projectedVertex,
                                       float combinedMatrix[4][4]) {
  float vec4[4] = {vertex[0], vertex[1], vertex[2], 1.0f};

  float clipSpaceVertex[4];
  for (int i = 0; i < 4; ++i) {
    clipSpaceVertex[i] =
        combinedMatrix[i][0] * vec4[0] + combinedMatrix[i][1] * vec4[1] +
        combinedMatrix[i][2] * vec4[2] + combinedMatrix[i][3] * vec4[3];
  }
  projectedVertex[0] = (clipSpaceVertex[0] / clipSpaceVertex[3] + 1.0) / 2.0;
  projectedVertex[1] = (clipSpaceVertex[1] / clipSpaceVertex[3] + 1.0) / 2.0;
  projectedVertex[2] = clipSpaceVertex[2] / clipSpaceVertex[3];
}

static __device__ float getTriangleZ(float px, float py,
                                     const float *projectedVertices) {
  float x1 = projectedVertices[0];
  float y1 = projectedVertices[1];
  float x2 = projectedVertices[3];
  float y2 = projectedVertices[4];
  float x3 = projectedVertices[6];
  float y3 = projectedVertices[7];
  float lambda1 = ((y2 - y3) * (px - x3) + (x3 - x2) * (py - y3)) /
                  ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
  float lambda2 = ((y3 - y1) * (px - x3) + (x1 - x3) * (py - y3)) /
                  ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
  float lambda3 = 1 - lambda1 - lambda2;
  if (lambda1 >= 0 && lambda2 >= 0 && lambda3 >= 0 && lambda1 <= 1 &&
      lambda2 <= 1 && lambda3 <= 1) {
    float result = lambda1 * projectedVertices[2] +
                   lambda2 * projectedVertices[5] +
                   lambda3 * projectedVertices[8];
    return result;
  } else {
    return -1;
  }
}

static __device__ void rasterization(int numTriangles,
                                     const float *projectedVertices,
                                     const float *vertices, const float *colors,
                                     float *outColor, float x, float y) {
  float minZ = FLT_MAX;
  int renderedIdx = -1;
  for (int i = 0; i < numTriangles; i++) {
    float z = getTriangleZ(x, y, projectedVertices + i * 9);
    if (z < 0) {
      // not in triangle
      continue;
    }
    if (z < minZ) {
      minZ = z;
      renderedIdx = i;
    }
  }
  if (renderedIdx != -1) {
    outColor[0] = colors[renderedIdx * 4];
    outColor[1] = colors[renderedIdx * 4 + 1];
    outColor[2] = colors[renderedIdx * 4 + 2];
    outColor[3] = colors[renderedIdx * 4 + 3];
  }
}

__global__ void kernelProjectVertices(float *combinedMatrix,
                                      float *projectedVertices,
                                      int numTriangles, float *vertices) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  __shared__ float sCombinedMatrix[4][4];

  if (threadIdx.x < 16) {
    sCombinedMatrix[threadIdx.x / 4][threadIdx.x % 4] =
        combinedMatrix[threadIdx.x];
  }
  __syncthreads();

  // project vertices
  if (idx < numTriangles * 9) {
    transformVertex(&vertices[3 * idx], projectedVertices + idx * 3,
                    sCombinedMatrix);
    // printf("original vertices: %f %f %f\n",
    //        cuConstRendererParams.vertices[3 * idx],
    //        cuConstRendererParams.vertices[3 * idx + 1],
    //        cuConstRendererParams.vertices[3 * idx + 2]);
    // printf("projectedVertices: %f %f %f\n", projectedVertices[idx * 3],
    //        projectedVertices[idx * 3 + 1], projectedVertices[idx * 3 + 2]);
  }
}

// render the image by pixels
__global__ void kernelRenderPixels(int batchIdx, float *projectedVertices,
                                   float *deviceImageData, int numTriangles,
                                   int imageWidth, int imageHeight,
                                   float *vertices, float *colors) {
  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y =
      batchIdx * blockDim.y * gridDim.y + blockIdx.y * blockDim.y + threadIdx.y;

  // rasterization
  float px = (x + 0.5f) / imageWidth;
  float py = (y + 0.5f) / imageHeight;
  if (x < imageWidth && y < imageHeight) {
    rasterization(numTriangles, projectedVertices, vertices, colors,
                  &deviceImageData[4 * (y * imageWidth + x)], px, py);
  }
}

////////////////////////////////////////////////////////////////////////////////////////

CudaRenderer::CudaRenderer(int numProcs, int numBatches)
    : numProcs(numProcs), numBatches(numBatches) {
  image = NULL;

  // cudaDeviceVertices = NULL;
  // cudaDeviceEndIndices = NULL;
  // cudaDeviceColors = NULL;
}

CudaRenderer::~CudaRenderer() {

  if (image) {
    delete image;
  }

  // if (cudaDeviceVertices) {
  //   cudaFree(cudaDeviceVertices);
  //   cudaFree(cudaDeviceEndIndices);
  //   cudaFree(cudaDeviceColors);
  // }
}

const Image *CudaRenderer::getImage() {

  // Need to copy contents of the rendered image from device memory
  // before we expose the Image object to the caller

  printf("Copying image data from device\n");

  // cudaMemcpy(image->data, cudaDeviceImageData,
  //            sizeof(float) * 4 * image->width * image->height,
  //            cudaMemcpyDeviceToHost);

  return image;
}

void CudaRenderer::loadScene(SceneName name) {
  sceneName = name;
  scene = ::loadScene(sceneName, image->width, image->height);
  scene->serialize(numTriangles, vertices, colors);
}

void CudaRenderer::setup() {
  int deviceCount = 0;
  std::string name;
  cudaError_t err = cudaGetDeviceCount(&deviceCount);

  printf("---------------------------------------------------------\n");
  printf("Initializing CUDA for CudaRenderer\n");
  printf("Found %d CUDA devices\n", deviceCount);

  for (int i = 0; i < deviceCount; i++) {
    cudaSetDevice(i);
    cudaDeviceProp deviceProps;
    cudaGetDeviceProperties(&deviceProps, i);
    name = deviceProps.name;

    printf("Device %d: %s\n", i, deviceProps.name);
    printf("   SMs:        %d\n", deviceProps.multiProcessorCount);
    printf("   Global mem: %.0f MB\n",
           static_cast<float>(deviceProps.totalGlobalMem) / (1024 * 1024));
    printf("   CUDA Cap:   %d.%d\n", deviceProps.major, deviceProps.minor);
  }
  printf("---------------------------------------------------------\n");
  cudaSetDevice(0);

  // cudaMalloc(&cudaDeviceVertices, sizeof(float) * 9 * numTriangles);
  // cudaMalloc(&cudaDeviceColors, sizeof(float) * 4 * numTriangles);
  // cudaMalloc(&cudaDeviceImageData,
  //            sizeof(float) * 4 * image->width * image->height);
  // cudaMalloc(&deviceCombinedMatrix, sizeof(float) * 16);
  // cudaMalloc(&projectedVertices, sizeof(float) * 9 * numTriangles);

  // cudaMemcpy(cudaDeviceVertices, vertices, sizeof(float) * 9 * numTriangles,
  //            cudaMemcpyHostToDevice);
  // cudaMemcpy(cudaDeviceColors, colors, sizeof(float) * 4 * numTriangles,
  //            cudaMemcpyHostToDevice);

  // GlobalConstants params;
  // params.sceneName = sceneName;
  // params.numTriangles = numTriangles;
  // params.vertices = cudaDeviceVertices;
  // params.colors = cudaDeviceColors;
  // params.imageWidth = image->width;
  // params.imageHeight = image->height;
  // params.imageData = cudaDeviceImageData;

  // cudaMemcpyToSymbol(cuConstRendererParams, &params,
  // sizeof(GlobalConstants));

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
  scene->cameraRotator.rotateCamera(scene->camera);
}

void CudaRenderer::render() {
  float combinedMatrix[4][4];
  scene->camera.calculateViewMatrix(combinedMatrix);
  // printf("Combined Matrix: \n");
  // for (int i = 0; i < 4; ++i) {
  //   printf("%f, %f, %f, %f\n", combinedMatrix[i][0], combinedMatrix[i][1],
  //          combinedMatrix[i][2], combinedMatrix[i][3]);
  // }
  float *deviceCombinedMatrix;
  float *projectedVertices;
  float *deviceVertices;
  cudaMalloc(&deviceCombinedMatrix, sizeof(float) * 16);
  cudaMalloc(&deviceVertices, sizeof(float) * 9 * numTriangles);
  cudaMalloc(&projectedVertices, sizeof(float) * 9 * numTriangles);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      cudaMemcpy(deviceCombinedMatrix + i * 4 + j, &combinedMatrix[i][j],
                 sizeof(float), cudaMemcpyHostToDevice);
    }
  }
  cudaMemcpy(deviceVertices, vertices, sizeof(float) * 9 * numTriangles,
             cudaMemcpyHostToDevice);

  dim3 blockDim(BLOCK_WIDTH * BLOCK_WIDTH);
  dim3 gridDim(
      ceil((double)(numTriangles * 9) / (double)(BLOCK_WIDTH * BLOCK_WIDTH)));
  kernelProjectVertices<<<gridDim, blockDim>>>(
      deviceCombinedMatrix, projectedVertices, numTriangles, deviceVertices);
  cudaCheckError(cudaDeviceSynchronize());

  float *hostProjectedVertices = new float[numTriangles * 9];
  cudaMemcpy(hostProjectedVertices, projectedVertices,
             numTriangles * 9 * sizeof(float), cudaMemcpyDeviceToHost);

  cudaFree(deviceCombinedMatrix);
  cudaFree(projectedVertices);
  cudaFree(deviceVertices);
  // for (int i = 0; i < numTriangles; i++) {
  //   printf("Triangle %d: ", i);
  //   for (int j = 0; j < 3; j++) {
  //     printf("%f, %f, %f", hostProjectedVertices[i * 9 + j * 3],
  //            hostProjectedVertices[i * 9 + j * 3 + 1],
  //            hostProjectedVertices[i * 9 + j * 3 + 2]);
  //   }
  //   printf("\n");
  // }
  int numGPUs = 0;
  cudaGetDeviceCount(&numGPUs);
  omp_set_num_threads(numGPUs);
#pragma omp parallel for default(shared) schedule(static)
  for (int i = 0; i < numGPUs; i++) {
    cudaSetDevice(i);
    // cudaSetDevice(0);
    int imageWidth = image->width;
    int imageHeight = image->height;

    float *cudaDeviceVertices;
    float *cudaDeviceColors;
    float *cudaDeviceImageData;
    float *deviceCombinedMatrix;
    float *deviceProjectedVertices;
    float *deviceImageData;
    cudaMalloc(&cudaDeviceVertices, sizeof(float) * 9 * numTriangles);
    cudaMalloc(&cudaDeviceColors, sizeof(float) * 4 * numTriangles);
    cudaMalloc(&cudaDeviceImageData,
               sizeof(float) * 4 * image->width * image->height);
    cudaMalloc(&deviceCombinedMatrix, sizeof(float) * 16);
    cudaMalloc(&deviceProjectedVertices, numTriangles * 9 * sizeof(float));
    cudaMalloc(&deviceImageData, imageWidth * imageHeight * 4 * sizeof(float));

    cudaMemcpy(cudaDeviceVertices, vertices, sizeof(float) * 9 * numTriangles,
               cudaMemcpyHostToDevice);
    cudaMemcpy(cudaDeviceColors, colors, sizeof(float) * 4 * numTriangles,
               cudaMemcpyHostToDevice);
    cudaMemcpy(deviceProjectedVertices, hostProjectedVertices,
               numTriangles * 9 * sizeof(float), cudaMemcpyHostToDevice);
    for (int ii = 0; ii < 4; ii++) {
      for (int j = 0; j < 4; j++) {
        cudaMemcpy(deviceCombinedMatrix + ii * 4 + j, &combinedMatrix[ii][j],
                   sizeof(float), cudaMemcpyHostToDevice);
      }
    }
    dim3 blockDim(16, 16, 1);
    dim3 gridDim((image->width + blockDim.x - 1) / blockDim.x,
                 (image->height + blockDim.y - 1) / blockDim.y);
    kernelClearImage<<<gridDim, blockDim>>>(deviceImageData, imageWidth,
                                            imageHeight, 1.f, 1.f, 1.f, 1.f);
    cudaDeviceSynchronize();

    blockDim = dim3(BLOCK_WIDTH, BLOCK_WIDTH);
    gridDim =
        dim3(ceil((double)imageWidth / (double)BLOCK_WIDTH),
             ceil((double)imageHeight / (double)BLOCK_WIDTH / (double)numGPUs));
    kernelRenderPixels<<<gridDim, blockDim>>>(
        i, deviceProjectedVertices, deviceImageData, numTriangles, imageWidth,
        imageHeight, cudaDeviceVertices, cudaDeviceColors);
    cudaCheckError(cudaDeviceSynchronize());

    int offset = i * (imageHeight / numGPUs) * imageWidth * 4;
    cudaMemcpy(image->data + offset, deviceImageData + offset,
               sizeof(float) * 4 * imageWidth * (imageHeight / numGPUs),
               cudaMemcpyDeviceToHost);

    cudaFree(cudaDeviceVertices);
    cudaFree(cudaDeviceColors);
    cudaFree(cudaDeviceImageData);
    cudaFree(deviceCombinedMatrix);
    cudaFree(deviceProjectedVertices);
    cudaFree(deviceImageData);
  }

  delete[] hostProjectedVertices;
}
