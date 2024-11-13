#include "camera.h"
#include "polygon.h"

class Scene {
 public:
  std::vector<Polygon> polygons;
  Camera camera;

  Scene();
  Scene(const std::vector<Polygon>& objs, const Camera& cam);

  void serialize(int& numPolygons, int& numVertices, float* vertices,
                 int* endIndices, float* colors);
};