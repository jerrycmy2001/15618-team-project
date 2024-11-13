#ifndef __CIRCLE_RENDERER_H__
#define __CIRCLE_RENDERER_H__

struct Image;

// fireworks constants
#define NUM_FIREWORKS 15
#define NUM_SPARKS 20

typedef enum { SIMPLE_TRIANGLE } SceneName;

class Renderer {
 protected:
  SceneName sceneName;
  int numPolygons;
  int numVertices;
  float *vertices;
  int *endIndices;
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
