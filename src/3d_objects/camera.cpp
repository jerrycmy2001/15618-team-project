#include "camera.h"

#include "Vector3.h"

Camera::Camera() {}

Camera::Camera(const Vector3& pos, const Vector3& dir, float fov)
    : position(pos), direction(dir.normalize()), fieldOfView(fov) {}

void Camera::updatePosition(const Vector3& newPos) { position = newPos; }
void Camera::updateDirection(const Vector3& newDir) { direction = newDir; }
void Camera::updateFieldOfView(const float newFov) { fieldOfView = newFov; }

void Camera::updateProjectionMatrix(const float windowWidth,
                                    const float windowHeight) {
  float fov = glm::radians(fieldOfView);  // Field of view, converted to radians
  float aspectRatio = windowWidth / windowHeight;
  float nearPlane = 0.1f;    // Near clipping plane
  float farPlane = 1000.0f;  // Far clipping plane

  // Create a perspective projection matrix
  projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
}