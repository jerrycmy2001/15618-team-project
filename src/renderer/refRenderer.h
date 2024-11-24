#ifndef __REF_RENDERER_H__
#define __REF_RENDERER_H__

#include "3d_object.h"
#include "renderer.h"

class RefRenderer : public Renderer {
 private:
  Image* image;
  SceneName sceneName;

 public:
  RefRenderer();
  virtual ~RefRenderer();

  const Image* getImage();

  void setup();

  void loadScene(SceneName name);

  void allocOutputImage(int width, int height);

  void clearImage();

  void advanceAnimation();

  void render();

  void shadePixel(float pixelCenterX, float pixelCenterY, float px, float py,
                  float pz, float* pixelData, int circleIndex);

  Triangle* initializePolygons();

  Vector3 transformVertex(const Vector3& vertex, float combinedMatrix[4][4]);
};

#endif
