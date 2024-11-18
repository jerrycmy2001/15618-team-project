#include "3d_object.h"

Triangle::Triangle() {}

Triangle::Triangle(const std::array<Vector3, 3>& verts,
                   const std::array<float, 4>& color)
    : vertices(verts), color(color) {}

void Triangle::setColor(const std::array<float, 4>& newcolor) {
  color = newcolor;
}

std::vector<Triangle> Triangle::getTriangles() const {
  return {Triangle(*this)};
}

Square::Square(const std::array<Vector3, 3>& verts,
               const std::array<float, 4>& color)
    : vertices(verts), color(color) {}

std::vector<Triangle> Square::getTriangles() const {
  Vector3 a = vertices[0], b = vertices[1], c = vertices[2],
          d = vertices[2] + vertices[1] - vertices[0];
  return {Triangle({a, b, c}, color), Triangle({b, c, d}, color)};
}