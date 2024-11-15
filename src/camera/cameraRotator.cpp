#include "cameraRotator.h"

CameraRotator::CameraRotator() : axisCenter(), axisDirection(), angleSpeed() {}

CameraRotator::CameraRotator(const Vector3& axisCenter,
                             const Vector3& axisDirection, float angleSpeed)
    : axisCenter(axisCenter),
      axisDirection(axisDirection),
      angleSpeed(angleSpeed) {}

void CameraRotator::rotateCamera(Camera& cam) const {
  // TODO: rotate camera
}