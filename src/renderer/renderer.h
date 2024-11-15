#ifndef __CIRCLE_RENDERER_H__
#define __CIRCLE_RENDERER_H__

#include <memory>

#include "scene.h"

struct Image;

// fireworks constants
#define NUM_FIREWORKS 15
#define NUM_SPARKS 20

typedef enum {
  SINGLE_TRIANGLE,
  DOUBLE_OVERLAPPING_TRIANGLES,
  NON_ORTHOGONAL_TRIANGLES
} SceneName;

class Renderer {
 protected:
  SceneName sceneName;
  std::shared_ptr<Scene> scene;
  int numTriangles;
  float *vertices;
  float *colors;

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
