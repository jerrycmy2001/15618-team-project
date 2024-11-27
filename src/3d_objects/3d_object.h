#pragma once

#include <array>
#include <ctime>
#include <vector>

#include "Vector3.h"

class Triangle;

class Object3D {
 public:
  virtual std::vector<Triangle> getTriangles() const = 0;

  virtual ~Object3D(){};
};

class Triangle : public Object3D {
 public:
  std::array<Vector3, 3> vertices;
  std::array<float, 4> color;  // r g b a; 0-255 for rgb, 0-1 for a

  Triangle();
  Triangle(const std::array<Vector3, 3>& verts,
           const std::array<float, 4>& color);
  void setColor(const std::array<float, 4>& color);
  std::vector<Triangle> getTriangles() const override;
};

class Square : public Object3D {
 public:
  std::array<Vector3, 3> vertices;  // a, b, c (d = b + c - a)
  std::array<float, 4> color;       // r g b a; 0-255 for rgb, 0-1 for a

  Square(const std::array<Vector3, 3>& verts,
         const std::array<float, 4>& color);
  std::vector<Triangle> getTriangles() const override;
};

class Cube : public Object3D {
 public:
  std::array<Vector3, 4> vertices;  // a, b, c, d, e = b + c - a, f = b + d - a,
                                    // g = c + d - a, h = c + f - a
  std::array<std::array<float, 4>, 6>
      color;  // r g b a; 0-255 for rgb, 0-1 for a

  Cube(const std::array<Vector3, 4>& verts,
       const std::array<std::array<float, 4>, 6>&
           color);  // vertices: 0-1, 0-2, 0-3; colors: 0-1-2, 0-1-3, 0-2-3 and
  // their opposites
  Cube(const std::array<Vector3, 2>& verts, const Vector3& direction,
       const std::array<std::array<float, 4>, 6>& color);
  Cube(float x_min, float x_max, float y_min, float y_max, float z_min,
       float z_max);
  std::vector<Triangle> getTriangles() const override;
};

class RegularTetrahedron : public Object3D {
 public:
  std::array<Vector3, 4> vertices;  // a, b, c
  std::array<std::array<float, 4>, 4>
      color;  // r g b a; 0-255 for rgb, 0-1 for a

  RegularTetrahedron(const std::array<Vector3, 4>& verts,
                     const std::array<std::array<float, 4>, 4>& color);
  RegularTetrahedron(const Vector3& vertexA, float edgeLength,
                     const Vector3& heightDirection,
                     const Vector3& bottomDirection,
                     const std::array<std::array<float, 4>, 4>& color);
  RegularTetrahedron(float x_min, float x_max, float y_min, float y_max,
                     float z_min, float z_max);
  std::vector<Triangle> getTriangles() const override;
};