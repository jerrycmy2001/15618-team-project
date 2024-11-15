#pragma once

#include "camera.h"
#include "cameraRotator.h"
#include "triangle.h"

class Scene {
 public:
  std::vector<Triangle> triangles;
  Camera camera;
  CameraRotator cameraRotator;

  Scene();
  Scene(const std::vector<Triangle>& triangles, const Camera& cam,
        const CameraRotator& camRotator);

  void serialize(int& numTriangles, float* vertices, float* colors);
};