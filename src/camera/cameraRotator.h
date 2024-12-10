#include <cmath>

#include "camera.h"
#include "vector3.h"

class CameraRotator {
 private:
  Vector3 axisCenter;
  Vector3 axisDirection;
  float angleSpeed;
  bool shouldRotate;

 public:
  CameraRotator();
  CameraRotator(const Vector3& axisCenter, const Vector3& axisDirection,
                float angleSpeed);
  CameraRotator(float min, float max);
  CameraRotator(const CameraRotator& c) = default;
  CameraRotator& operator=(const CameraRotator& c) = default;

  void rotateCamera(Camera& cam) const;
};