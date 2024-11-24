#include "cameraRotator.h"

#include <cstdio>

CameraRotator::CameraRotator()
    : axisCenter(), axisDirection(), angleSpeed(), shouldRotate(false) {}

CameraRotator::CameraRotator(const Vector3& axisCenter,
                             const Vector3& axisDirection, float angleSpeed)
    : axisCenter(axisCenter),
      axisDirection(axisDirection),
      angleSpeed(angleSpeed),
      shouldRotate(true) {}

void CameraRotator::rotateCamera(Camera& cam) const {
  if (!shouldRotate) {
    return;
  }
  // Normalize the axis of rotation
  Vector3 k = axisDirection.normalize();

  // Calculate the angle in radians
  float theta = angleSpeed;  // Assuming angleSpeed is in radians per update
  float cosTheta = cos(theta);
  float sinTheta = sin(theta);

  // Construct the skew-symmetric matrix for k
  float K[3][3] = {{0, -k.z, k.y}, {k.z, 0, -k.x}, {-k.y, k.x, 0}};

  // Compute K^2
  float K2[3][3];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      K2[i][j] = 0;
      for (int m = 0; m < 3; ++m) {
        K2[i][j] += K[i][m] * K[m][j];
      }
    }
  }

  // Compute the rotation matrix R
  float R[3][3];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      R[i][j] = (i == j) + sinTheta * K[i][j] + (1 - cosTheta) * K2[i][j];
    }
  }

  // Rotate camera position
  Vector3 pos = cam.position - axisCenter;  // Translate to origin
  Vector3 rotatedPos(0, 0, 0);
  rotatedPos.x += R[0][0] * pos.x + R[0][1] * pos.y + R[0][2] * pos.z;
  rotatedPos.y += R[1][0] * pos.x + R[1][1] * pos.y + R[1][2] * pos.z;
  rotatedPos.z += R[2][0] * pos.x + R[2][1] * pos.y + R[2][2] * pos.z;
  rotatedPos = rotatedPos + axisCenter;  // Translate back

  // Rotate camera direction
  Vector3 rotatedDir(0, 0, 0);
  rotatedDir.x += R[0][0] * cam.direction.x + R[0][1] * cam.direction.y +
                  R[0][2] * cam.direction.z;
  rotatedDir.y += R[1][0] * cam.direction.x + R[1][1] * cam.direction.y +
                  R[1][2] * cam.direction.z;
  rotatedDir.z += R[2][0] * cam.direction.x + R[2][1] * cam.direction.y +
                  R[2][2] * cam.direction.z;

  // Rotate camera up
  Vector3 rotatedUp(0, 0, 0);
  rotatedUp.x += R[0][0] * cam.up.x + R[0][1] * cam.up.y + R[0][2] * cam.up.z;
  rotatedUp.y += R[1][0] * cam.up.x + R[1][1] * cam.up.y + R[1][2] * cam.up.z;
  rotatedUp.z += R[2][0] * cam.up.x + R[2][1] * cam.up.y + R[2][2] * cam.up.z;

  // Update camera
  cam.updateStatus(rotatedPos, rotatedDir, rotatedUp);
}
