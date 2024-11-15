#ifndef CAMERA_H
#define CAMERA_H

#include "vector3.h"

class Camera {
 public:
  Vector3 position;
  Vector3 direction;
  Vector3 up;
  float fieldOfView;  // in degrees
  float closeDistance;
  float farDistance;
  int windowHeight;
  int windowWidth;

  Camera();
  Camera(const Vector3& pos, const Vector3& dir, const Vector3& up, float fov,
         float close, float far, int w, int h);
  Camera(const Camera& c) = default;
  Camera& operator=(const Camera& c) = default;

  void updateStatus(const Vector3& newPos, const Vector3& newDir,
                    const Vector3& newUp);

  void initCameraMatrix(float matrix[4][4]) const;
  void initProjectionMatrix(float result[4][4]) const;
  void calculateViewMatrix(float matrix[4][4]) const;
};

#endif
