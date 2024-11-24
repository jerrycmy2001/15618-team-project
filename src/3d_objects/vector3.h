#ifndef VECTOR3_H
#define VECTOR3_H

class Vector3 {
 public:
  float x, y, z;

  Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f);

  Vector3(const Vector3& v) = default;
  Vector3& operator=(const Vector3& v) = default;
  Vector3(Vector3&& v) = default;
  Vector3& operator=(Vector3&& v) = default;
  ~Vector3() = default;

  Vector3 operator+(const Vector3& v) const;
  Vector3 operator-(const Vector3& v) const;
  Vector3 operator*(float s) const;
  Vector3 operator/(float s) const;

  float magnitude() const;
  Vector3 normalize() const;
  Vector3 cross(const Vector3& v) const;
  float dot(const Vector3& v) const;
  float length() const;
  bool isZero() const;
};

#endif