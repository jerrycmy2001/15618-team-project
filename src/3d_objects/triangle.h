#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <array>
#include <vector>

#include "Vector3.h"

class Triangle {
 public:
  std::array<Vector3, 3> vertices;
  std::array<float, 4> color;  // r g b a; 0-255 for rgb, 0-1 for a

  Triangle();
  Triangle(const std::array<Vector3, 3>& verts,
           const std::array<float, 4>& color);
  void setColor(const std::array<float, 4>& color);
};

#endif
