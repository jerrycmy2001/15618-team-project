#ifndef __SCENE_LOADER_H__
#define __SCENE_LOADER_H__

#include "renderer.h"
#include "scene.h"

void loadCircleScene(SceneName sceneName, int& numPolygons, int& numVertices,
                     float* vertices, int* endIndices, float* colors);

#endif
