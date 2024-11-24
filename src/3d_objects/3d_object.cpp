#include "3d_object.h"

#include <cmath>

#include "Vector3.h"

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

Cube::Cube(const std::array<Vector3, 4>& verts,
           const std::array<std::array<float, 4>, 6>& color)
    : vertices(verts), color(color) {}

Cube::Cube(const std::array<Vector3, 2>& verts, const Vector3& direction,
           const std::array<std::array<float, 4>, 6>& color) {
  std::array<Vector3, 4> newVerts;
  newVerts[0] = verts[0];
  newVerts[1] = verts[1];

  Vector3 edge1 = verts[1] - verts[0];
  Vector3 perpendicularForV2 = edge1.cross(direction).normalize();

  if (perpendicularForV2.isZero()) {
    // direction is parallel to edge1, generate a new direction
    Vector3 newDir(direction.x, direction.y, direction.z + 1.0);
    perpendicularForV2 = edge1.cross(newDir).normalize();
  }

  float sideLength = edge1.magnitude();
  Vector3 scaledPerpendicularForV2 = perpendicularForV2 * sideLength;

  newVerts[2] = verts[0] + scaledPerpendicularForV2;

  Vector3 perpendicularForV3 = edge1.cross(perpendicularForV2).normalize();
  Vector3 scaledPerpendicularForV3 = perpendicularForV3 * sideLength;

  newVerts[3] = verts[0] + scaledPerpendicularForV3;

  Cube(newVerts, color);
}

std::vector<Triangle> Cube::getTriangles() const {
  Vector3 a = vertices[0], b = vertices[1], c = vertices[2], d = vertices[3],
          e = b + c - a, f = b + d - a, g = c + d - a, h = c + f - a;
  return {Triangle({a, b, c}, color[0]), Triangle({b, c, e}, color[0]),
          Triangle({a, b, d}, color[1]), Triangle({b, d, f}, color[1]),
          Triangle({a, c, d}, color[2]), Triangle({c, d, g}, color[2]),
          Triangle({d, f, h}, color[3]), Triangle({d, h, g}, color[3]),
          Triangle({c, f, h}, color[4]), Triangle({f, h, e}, color[4]),
          Triangle({b, e, h}, color[5]), Triangle({e, h, f}, color[5])};
}

RegularTetrahedron::RegularTetrahedron(
    const std::array<Vector3, 4>& verts,
    const std::array<std::array<float, 4>, 4>& color) {
  vertices = verts;
  this->color = color;
}

RegularTetrahedron::RegularTetrahedron(
    const Vector3& A, const Vector3& heightDirection,
    const Vector3& bottomDirection,
    const std::array<std::array<float, 4>, 4>& color) {
  Vector3 G = A + heightDirection;

  Vector3 X = heightDirection.cross(bottomDirection).normalize();
  Vector3 Y = heightDirection.cross(X).normalize();

  double H = heightDirection.length();
  double l = l / std::sqrt(2.0);

  Vector3 B = G + X * H;
  Vector3 C = B - X * H / 2 + Y * l * std::sqrt(3) / 2;
  Vector3 D = B - X * H / 2 - Y * l * std::sqrt(3) / 2;

  vertices = {A, B, C, D};

  this->color = color;
}

std::vector<Triangle> RegularTetrahedron::getTriangles() const {
  Vector3 A = vertices[0], B = vertices[1], C = vertices[2], D = vertices[3];
  return {Triangle({A, B, C}, color[0]), Triangle({A, B, D}, color[1]),
          Triangle({A, C, D}, color[2]), Triangle({B, C, D}, color[3])};
}