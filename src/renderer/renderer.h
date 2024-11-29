#ifndef __CIRCLE_RENDERER_H__
#define __CIRCLE_RENDERER_H__

#include <memory>

#include "scene.h"

struct Image;

// fireworks constants
#define NUM_FIREWORKS 15
#define NUM_SPARKS 20
class Renderer {
 protected:
  std::shared_ptr<Scene> scene;
  int numTriangles;
  float *vertices;
  float *colors;
  SceneName sceneName;

 public:
  virtual ~Renderer(){};

  virtual const Image *getImage() = 0;

  virtual void setup() = 0;

  virtual void loadScene(SceneName name) = 0;

  virtual void allocOutputImage(int width, int height) = 0;

  virtual void clearImage() = 0;

  virtual void advanceAnimation() = 0;

  virtual void render() = 0;
};

#endif
