#include "triangle.h"

Triangle::Triangle() {}

Triangle::Triangle(const std::array<Vector3, 3>& verts,
                   const std::array<float, 4>& color)
    : vertices(verts), color(color) {}

void Triangle::setColor(const std::array<float, 4>& newcolor) {
  color = newcolor;
}
