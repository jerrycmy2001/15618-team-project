#include "camera.h"

#include <cmath>
#include <cstdio>

#include "Vector3.h"

Camera::Camera() {}

Camera::Camera(const Vector3& pos, const Vector3& dir, const Vector3& up,
               float fov, float close, float far, int w, int h)
    : position(pos),
      direction(dir.normalize()),
      up(up.normalize()),
      fieldOfView(fov),
      closeDistance(close),
      farDistance(far),
      windowHeight(h),
      windowWidth(w) {}

Camera::Camera(float min, float max, int w, int h) {
  float range = max - min;

  // Position
  float pos_x = static_cast<float>(rand()) / RAND_MAX * range + min;
  float pos_y = static_cast<float>(rand()) / RAND_MAX * range + min;
  float pos_z = static_cast<float>(rand()) / RAND_MAX * range + min;
  Vector3 pos = Vector3(pos_x, pos_y, pos_z);

  // Direction
  float dir_x = static_cast<float>(rand()) / RAND_MAX * range;
  float dir_y = static_cast<float>(rand()) / RAND_MAX * range;
  float dir_z = static_cast<float>(rand()) / RAND_MAX * range;
  while (pos_x == dir_x && pos_y == dir_y && pos_z == dir_z) {
    dir_z = static_cast<float>(rand()) / RAND_MAX * range;
  }
  Vector3 dir = Vector3(dir_x, dir_y, dir_z);

  // Up
  float up_x = static_cast<float>(rand()) / RAND_MAX * range;
  float up_y = static_cast<float>(rand()) / RAND_MAX * range;
  float up_z = static_cast<float>(rand()) / RAND_MAX * range;
  Vector3 up = Vector3(up_x, up_y, up_z);
  while (up.cross(direction).normalize().isZero()) {
    up_z = static_cast<float>(rand()) / RAND_MAX * range;
    up = Vector3(up_x, up_y, up_z);
  }

  float fov = static_cast<float>(rand()) / RAND_MAX * 180.0f;
  float close = static_cast<float>(rand()) / RAND_MAX * 10.0f;
  float far = static_cast<float>(rand()) / RAND_MAX * 10.0f + 100.0f;

  Camera(pos, dir, up, fov, close, far, w, h);
}

void Camera::updateStatus(const Vector3& newPos, const Vector3& newDir,
                          const Vector3& newUp) {
  position = newPos;
  direction = newDir.normalize();
  up = newUp.normalize();
}

// Matrix multiplication
void multiplyMatrices(float result[4][4], const float a[4][4],
                      const float b[4][4]) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      result[i][j] = 0.0f;
      for (int k = 0; k < 4; k++) {
        result[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

// Initialize the model matrix
void initModelMatrix(float matrix[4][4]) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      matrix[i][j] = (i == j) ? 1.0f : 0.0f;
    }
  }
}

// Initialize the camera matrix
void Camera::initCameraMatrix(float result[4][4]) const {
  Vector3 zaxis = direction;                    // Forward
  Vector3 xaxis = zaxis.cross(up).normalize();  // Right
  Vector3 yaxis = zaxis.cross(xaxis);           // Up
  // Vector3 yaxis = xaxis.cross(zaxis);  // Up

  result[0][0] = xaxis.x;
  result[0][1] = xaxis.y;
  result[0][2] = xaxis.z;
  result[0][3] = -xaxis.dot(position);
  result[1][0] = yaxis.x;
  result[1][1] = yaxis.y;
  result[1][2] = yaxis.z;
  result[1][3] = -yaxis.dot(position);
  result[2][0] = zaxis.x;
  result[2][1] = zaxis.y;
  result[2][2] = zaxis.z;
  result[2][3] = -zaxis.dot(position);
  result[3][0] = 0;
  result[3][1] = 0;
  result[3][2] = 0;
  result[3][3] = -1;
}

// Initialize projection matrix
void Camera::initProjectionMatrix(float result[4][4]) const {
  float tanHalfFOV = tan(fieldOfView / 2.0f);
  float zRange = closeDistance - farDistance;

  result[0][0] = 1.0f / (tanHalfFOV * (1.0f * windowWidth / windowHeight));
  result[0][1] = 0;
  result[0][2] = 0;
  result[0][3] = 0;
  result[1][0] = 0;
  result[1][1] = 1.0f / tanHalfFOV;
  result[1][2] = 0;
  result[1][3] = 0;
  result[2][0] = 0;
  result[2][1] = 0;
  result[2][2] = (farDistance + closeDistance) / zRange;
  result[2][3] = 2 * farDistance * closeDistance / zRange;
  result[3][0] = 0;
  result[3][1] = 0;
  result[3][2] = -1;
  result[3][3] = 0;
}

void Camera::calculateViewMatrix(float matrix[4][4]) const {
  // TODO: calculate view matrix
  float modelMatrix[4][4];
  initModelMatrix(modelMatrix);

  float cameraMatrix[4][4];
  initCameraMatrix(cameraMatrix);
  // printf("Camera Matrix: \n");
  // for (int i = 0; i < 4; ++i) {
  //   printf("%f, %f, %f, %f\n", cameraMatrix[i][0], cameraMatrix[i][1],
  //          cameraMatrix[i][2], cameraMatrix[i][3]);
  // }

  float projectionMatrix[4][4];
  initProjectionMatrix(projectionMatrix);
  // printf("Projection Matrix: \n");
  // for (int i = 0; i < 4; ++i) {
  //   printf("%f, %f, %f, %f\n", projectionMatrix[i][0],
  //   projectionMatrix[i][1],
  //          projectionMatrix[i][2], projectionMatrix[i][3]);
  // }

  float tmp[4][4];
  multiplyMatrices(tmp, projectionMatrix, cameraMatrix);
  multiplyMatrices(matrix, tmp, modelMatrix);
}