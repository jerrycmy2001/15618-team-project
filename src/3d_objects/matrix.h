#include "camera.h"
#include "vector3.h"

void initObjectMatrix(float matrix[4][4]);

void initCameraMatrix(float result[4][4], const Camera& camera);

void initProjectionMatrix(float result[4][4], const Camera& camera);