#pragma once

#include <array>
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