#ifndef __CUDA_RENDERER_H__
#define __CUDA_RENDERER_H__

#include "renderer.h"

class CudaRenderer : public Renderer {
 private:
  int numProcs;
  int numBatches;
  Image* image;

  //   float* cudaDeviceVertices;
  //   int* cudaDeviceEndIndices;
  //   float* cudaDeviceColors;
  //   float* cudaDeviceImageData;
  //   float* deviceCombinedMatrix;
  //   float* projectedVertices;

 public:
  CudaRenderer(int numProcs, int numBatches);
  virtual ~CudaRenderer();

  const Image* getImage();

  void setup();

  void loadScene(SceneName name);

  void allocOutputImage(int width, int height);

  void clearImage();

  void advanceAnimation();

  void render();

  void shadePixel(float pixelCenterX, float pixelCenterY, float px, float py,
                  float pz, float* pixelData, int circleIndex);
};

#endif
