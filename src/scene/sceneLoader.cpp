#include "renderer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <vector>

#include "sceneLoader.h"
#include "util.h"

std::shared_ptr<Scene> loadScene(SceneName sceneName, int windowWidth,
                                 int windowHeight) {
  std::vector<Triangle> objs;
  Camera cam;
  CameraRotator camRotator;
  switch (sceneName) {
    case SceneName::SINGLE_TRIANGLE:
      objs = {Triangle(
          {Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0.5, sqrt(3) / 2.f, 0)},
          {255, 0, 0, 1})};
      cam = Camera({0.5, (float)sqrt(3) / 4.f, 2}, {0, 0, -1}, {0, 1, 0}, 90.0f,
                   1.0f, 10.0f, windowWidth, windowHeight);
      break;
    case SceneName::DOUBLE_OVERLAPPING_TRIANGLES:
      objs = {Triangle({Vector3(0, 0, 0), Vector3(1, 0, 0),
                        Vector3(0.5, sqrt(3) / 2.f, 0)},
                       {255, 0, 0, 1}),
              Triangle({Vector3(0.5, 0, -1), Vector3(1.5, 0, -1),
                        Vector3(1, sqrt(3) / 2.f, -1)},
                       {0, 255, 0, 1})};
      cam = Camera({0.5, (float)sqrt(3) / 4.f, 2}, {0, 0, -1}, {0, 1, 0}, 90.0f,
                   1.0f, 10.0f, windowWidth, windowHeight);
      break;
    case SceneName::NON_ORTHOGONAL_TRIANGLES:
      objs = {Triangle({Vector3(0, 0, 0), Vector3(1, 0, 0),
                        Vector3(0.5, sqrt(3) / 2.f, 0)},
                       {255, 0, 0, 1}),
              Triangle({Vector3(0.5, 0, -1), Vector3(1.5, 0, -1),
                        Vector3(1, sqrt(3) / 2.f, -1)},
                       {0, 255, 0, 1})};
      cam = Camera({0.5, (float)sqrt(3) / 4.f, 5}, {1, 2, -1}, {0, 1, 0}, 90.0f,
                   1.0f, 10.0f, windowWidth, windowHeight);
      break;
    default:
      fprintf(stderr, "Error: cann't load scene (unknown scene)\n");
      exit(1);
  }
  return std::make_shared<Scene>(objs, cam, camRotator);
}
