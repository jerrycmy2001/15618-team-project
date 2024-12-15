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
  } else if (name == "rand8") {
    return SceneName::RAND8;
  } else if (name == "rand27") {
    return SceneName::RAND27;
  } else if (name == "rand64") {
    return SceneName::RAND64;
  } else if (name == "rand125") {
    return SceneName::RAND125;
  } else if (name == "rand216") {
    return SceneName::RAND216;
  } else if (name == "rand343") {
    return SceneName::RAND343;
  } else if (name == "rand512") {
    return SceneName::RAND512;
  } else if (name == "rand729") {
    return SceneName::RAND729;
  } else if (name == "rand1000") {
    return SceneName::RAND1000;
  } else if (name == "rand3375") {
    return SceneName::RAND3375;
  } else if (name == "rand8000") {
    return SceneName::RAND8000;
  } else if (name == "title") {
    return SceneName::COURSE_TITLE;
  } else if (name == "illusion") {
    return SceneName::VISUAL_ILLUSION;
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