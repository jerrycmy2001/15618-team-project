#include "scene.h"

#include "triangle.h"

Scene::Scene() {}

Scene::Scene(const std::vector<Triangle>& objs, const Camera& cam,
             const CameraRotator& camRotator)
    : triangles(objs), camera(cam), cameraRotator(camRotator) {}

void Scene::serialize(int& numTriangles, float* vertices, float* colors) {
  numTriangles = triangles.size();

  vertices = new float[numTriangles * 3 * 3];
  colors = new float[numTriangles * 4];

  for (int i = 0; i < numTriangles; i++) {
    for (int j = 0; j < triangles[i].vertices.size(); j++) {
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