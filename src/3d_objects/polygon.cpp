#include "polygon.h"

Polygon::Polygon(const std::vector<Vector3>& verts,
                 const std::array<float, 4>& color)
    : vertices(verts), color(color) {}

void Polygon::setColor(const std::array<float, 4>& newcolor) {
  color = newcolor;
}
