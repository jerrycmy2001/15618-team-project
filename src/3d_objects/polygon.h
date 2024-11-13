#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <array>
#include <vector>

#include "Vector3.h"

class Polygon {
 public:
  std::vector<Vector3> vertices;
  std::array<float, 4> color;

  Polygon(const std::vector<Vector3>& verts, const std::array<float, 4>& color);
  void setColor(const std::array<float, 4>& color);
};

#endif
