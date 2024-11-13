#include "renderer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <vector>

#include "sceneLoader.h"
#include "util.h"

void loadCircleScene(SceneName sceneName, int& numPolygons, int& numVertices,
                     float* vertices, int* endIndices, float* colors) {
  Scene scene;
  if (sceneName == SIMPLE_TRIANGLE) {
    std::vector<Polygon> objs = {
        Polygon({Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)},
                {1.0f, 0.0f, 0.0f, 1.0f})};
    Camera cam = Camera({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, 90.0f);
    scene = Scene(objs, cam);
  } else {
    fprintf(stderr, "Error: cann't load scene (unknown scene)\n");
    exit(1);
  }
  scene.serialize(numPolygons, numVertices, vertices, endIndices, colors);
}
