#ifndef __SCENE_LOADER_H__
#define __SCENE_LOADER_H__

#include "renderer.h"
#include "scene.h"

std::shared_ptr<Scene> loadScene(SceneName sceneName, int windowWidth,
                                 int windowHeight);

#endif
