#include "scene.h"

Scene::Scene() {}

Scene::Scene(const std::vector<Polygon>& objs, const Camera& cam)
    : polygons(objs), camera(cam) {}

void Scene::serialize(int& numPolygons, int& numVertices, float* vertices,
                      int* endIndices, float* colors) {
  numPolygons = polygons.size();
  numVertices = 0;
  for (int i = 0; i < numPolygons; i++) {
    numVertices += polygons[i].vertices.size();
  }

  vertices = new float[numVertices * 3];
  endIndices = new int[numPolygons + 1];
  colors = new float[numPolygons * 4];

  int currentVertexIndex = 0;
  endIndices[0] = 0;
  for (int i = 0; i < numPolygons; i++) {
    endIndices[i + 1] = endIndices[i] + polygons[i].vertices.size();
    for (int j = 0; j < polygons[i].vertices.size(); j++) {
      vertices[currentVertexIndex * 3] = polygons[i].vertices[j].x;
      vertices[currentVertexIndex * 3 + 1] = polygons[i].vertices[j].y;
      vertices[currentVertexIndex * 3 + 2] = polygons[i].vertices[j].z;
    }
    for (int j = 0; j < polygons[i].color.size(); j++) {
      colors[i * 4] = polygons[i].color[0];
      colors[i * 4 + 1] = polygons[i].color[1];
      colors[i * 4 + 2] = polygons[i].color[2];
      colors[i * 4 + 3] = polygons[i].color[3];
    }
  }
}