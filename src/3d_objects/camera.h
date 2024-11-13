#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "vector3.h"

class Camera {
 public:
  Vector3 position;
  Vector3 direction;
  float fieldOfView;
  glm::mat4 projectionMatrix;

  Camera();
  Camera(const Vector3& pos, const Vector3& dir, float fov);
  Camera(const Camera& c) = default;
  Camera& operator=(const Camera& c) = default;
  void updatePosition(const Vector3& newPos);
  void updateDirection(const Vector3& newDir);
  void updateFieldOfView(const float newFov);

  void updateProjectionMatrix(const float windowWidth,
                              const float windowHeight);
};

#endif
