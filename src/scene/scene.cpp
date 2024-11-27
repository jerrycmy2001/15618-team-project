#include "scene.h"

#include <cstdio>

#include "3d_object.h"

SceneName getSceneFromName(std::string name) {
  if (name == "single") {
    return SceneName::SINGLE_TRIANGLE;
  } else if (name == "double") {
    return SceneName::DOUBLE_OVERLAPPING_TRIANGLES;
  } else if (name == "non-orthogonal") {
    return SceneName::NON_ORTHOGONAL_TRIANGLES;
  } else if (name == "square") {
    return SceneName::SQUARE;
  } else if (name == "cube") {
    return SceneName::CUBE;
  } else if (name == "tetrahedron") {
    return SceneName::TETRAHEDRON;
  } else if (name == "rand") {
    return SceneName::RAND;
  } else if (name == "title") {
    return SceneName::COURSE_TITLE;
  } else {
    printf("Unknown scene name: %s\n", name.c_str());
    exit(1);
  }
}

Scene::Scene() {}

Scene::Scene(const std::vector<Triangle>& objs, const Camera& cam,
             const CameraRotator& camRotator)
    : triangles(objs), camera(cam), cameraRotator(camRotator) {}

void Scene::serialize(int& numTriangles, float*& vertices, float*& colors) {
  numTriangles = triangles.size();

  vertices = new float[numTriangles * 3 * 3];
  colors = new float[numTriangles * 4];

  for (int i = 0; i < numTriangles; i++) {
    for (int j = 0; j < 3; j++) {
      vertices[i * 9 + j * 3] = triangles[i].vertices[j].x;
      vertices[i * 9 + j * 3 + 1] = triangles[i].vertices[j].y;
      vertices[i * 9 + j * 3 + 2] = triangles[i].vertices[j].z;
    }
    colors[i * 4] = triangles[i].color[0];
    colors[i * 4 + 1] = triangles[i].color[1];
    colors[i * 4 + 2] = triangles[i].color[2];
    colors[i * 4 + 3] = triangles[i].color[3];
  }
}