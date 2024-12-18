#include "3d_object.h"

#include <cmath>
#include <cstdio>

#include "vector3.h"

float getRandFloat01() { return (float)rand() / (float)RAND_MAX; }

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
  // printf("%d %d %d %d %d %d %d %d %d %d %d %d\n", (int)newVerts[0].x,
  //        (int)newVerts[0].y, (int)newVerts[0].z, (int)newVerts[1].x,
  //        (int)newVerts[1].y, (int)newVerts[1].z, (int)newVerts[2].x,
  //        (int)newVerts[2].y, (int)newVerts[2].z, (int)newVerts[3].x,
  //        (int)newVerts[3].y, (int)newVerts[3].z);

  *this = Cube(newVerts, color);
}

std::array<float, 4> getRandomColor() {
  return {getRandFloat01(), getRandFloat01(), getRandFloat01(),
          getRandFloat01()};
}

Cube::Cube(float x_min, float x_max, float y_min, float y_max, float z_min,
           float z_max) {
  float x_mid = (x_max + x_min) / 2.0f;
  float y_mid = (y_max + y_min) / 2.0f;
  float z_mid = (z_max + z_min) / 2.0f;
  float x_range = x_max - x_min;
  float y_range = y_max - y_min;
  float z_range = z_max - z_min;

  // Vertex
  float first_x = getRandFloat01() + (x_mid - 0.5f);
  float first_y = getRandFloat01() + (y_mid - 0.5f);
  float first_z = getRandFloat01() + (z_mid - 0.5f);
  Vector3 first = Vector3(first_x, first_y, first_z);

  // Second vertex
  float sdir_x = getRandFloat01() * x_range;
  float sdir_y = getRandFloat01() * y_range;
  float sdir_z = getRandFloat01() * z_range;
  Vector3 sdir = Vector3(sdir_x, sdir_y, sdir_z).normalize();

  // float max_edge_length = std::cbrt(x_mid - 0.5f - x_min);
  // float edge_length = getRandFloat01() * (max_edge_length - 1.0f) + 1.0f;
  float edge_length = getRandFloat01() * (x_mid - 0.5f - x_min - 1.0f) + 1.0f;
  Vector3 second = first + (sdir * edge_length);

  // Direction
  float dir_x = getRandFloat01() * x_range;
  float dir_y = getRandFloat01() * y_range;
  float dir_z = getRandFloat01() * z_range;
  Vector3 dir = Vector3(dir_x, dir_y, dir_z);

  *this = Cube({first, second}, dir,
               {
                   getRandomColor(),
                   getRandomColor(),
                   getRandomColor(),
                   getRandomColor(),
                   getRandomColor(),
                   getRandomColor(),
               });
}

std::vector<Triangle> Cube::getTriangles() const {
  Vector3 a = vertices[0], b = vertices[1], c = vertices[2], d = vertices[3],
          e = b + c - a, f = b + d - a, g = c + d - a, h = c + f - a;
  return {Triangle({a, b, c}, color[0]), Triangle({b, c, e}, color[0]),
          Triangle({a, b, d}, color[1]), Triangle({b, d, f}, color[1]),
          Triangle({a, c, d}, color[2]), Triangle({c, d, g}, color[2]),
          Triangle({d, f, h}, color[3]), Triangle({d, h, g}, color[3]),
          Triangle({c, e, h}, color[4]), Triangle({c, h, g}, color[4]),
          Triangle({b, e, h}, color[5]), Triangle({b, h, f}, color[5])};
}

RegularTetrahedron::RegularTetrahedron(
    const std::array<Vector3, 4>& verts,
    const std::array<std::array<float, 4>, 4>& color)
    : vertices(verts), color(color) {}

RegularTetrahedron::RegularTetrahedron(
    const Vector3& A, float edgeLength, const Vector3& heightDirection,
    const Vector3& bottomDirection,
    const std::array<std::array<float, 4>, 4>& color)
    : color(color) {
  float H = edgeLength * std::sqrt(2.0) / std::sqrt(3.0);
  Vector3 G = A + heightDirection.normalize() * H;

  Vector3 X = heightDirection.cross(bottomDirection).normalize();
  Vector3 Y = heightDirection.cross(X).normalize();

  double l = edgeLength / std::sqrt(3.0);

  Vector3 B = G + Y * l;
  Vector3 C = G - Y * l / 2 + X * l * std::sqrt(3) / 2;
  Vector3 D = G - Y * l / 2 - X * l * std::sqrt(3) / 2;

  vertices = {A, B, C, D};
  // printf("vertices: %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
  //        (int)vertices[0].x, (int)vertices[0].y, (int)vertices[0].z,
  //        (int)vertices[1].x, (int)vertices[1].y, (int)vertices[1].z,
  //        (int)vertices[2].x, (int)vertices[2].y, (int)vertices[2].z,
  //        (int)vertices[3].x, (int)vertices[3].y, (int)vertices[3].z);
}

RegularTetrahedron::RegularTetrahedron(float x_min, float x_max, float y_min,
                                       float y_max, float z_min, float z_max) {
  float x_mid = (x_max + x_min) / 2.0f;
  float y_mid = (y_max + y_min) / 2.0f;
  float z_mid = (z_max + z_min) / 2.0f;
  float x_range = x_max - x_min;
  float y_range = y_max - y_min;
  float z_range = z_max - z_min;

  // Vertex A
  float A_x = getRandFloat01() + (x_mid - 0.5f);
  float A_y = getRandFloat01() + (y_mid - 0.5f);
  float A_z = getRandFloat01() + (z_mid - 0.5f);
  Vector3 A(A_x, A_y, A_z);

  float edge_length = getRandFloat01() * (x_mid - 0.5f - x_min - 1.0f) + 1.0f;

  // Direction
  float hdir_x = getRandFloat01() * x_range;
  float hdir_y = getRandFloat01() * y_range;
  float hdir_z = getRandFloat01() * z_range;
  Vector3 hdir(hdir_x, hdir_y, hdir_z);

  float bdir_x = getRandFloat01() * x_range;
  float bdir_y = getRandFloat01() * y_range;
  float bdir_z = getRandFloat01() * z_range;
  Vector3 bdir(bdir_x, bdir_y, bdir_z);
  while (bdir.cross(hdir).isZero()) {
    bdir_z = getRandFloat01() * z_range;
    bdir = Vector3(bdir_x, bdir_y, bdir_z);
  }

  *this = RegularTetrahedron(
      A, edge_length, hdir, bdir,
      {getRandomColor(), getRandomColor(), getRandomColor(), getRandomColor()});
}

std::vector<Triangle> RegularTetrahedron::getTriangles() const {
  Vector3 A = vertices[0], B = vertices[1], C = vertices[2], D = vertices[3];
  return {Triangle({A, B, C}, color[0]), Triangle({A, B, D}, color[1]),
          Triangle({A, C, D}, color[2]), Triangle({B, C, D}, color[3])};
}