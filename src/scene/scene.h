#pragma once

#include <string>

#include "3d_object.h"
#include "camera.h"
#include "cameraRotator.h"

typedef enum {
  SINGLE_TRIANGLE,
  DOUBLE_OVERLAPPING_TRIANGLES,
  NON_ORTHOGONAL_TRIANGLES,
  SQUARE,
  CUBE,
  TETRAHEDRON,
  RAND,
  COURSE_TITLE,
} SceneName;

SceneName getSceneFromName(std::string name);

class Scene {
 public:
  std::vector<Triangle> triangles;
  Camera camera;
  CameraRotator cameraRotator;

  Scene();
  Scene(const std::vector<Triangle>& triangles, const Camera& cam,
        const CameraRotator& camRotator);

  void serialize(int& numTriangles, float*& vertices, float*& colors);
};