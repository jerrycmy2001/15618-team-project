#include <algorithm>
#include <limits>

#include "3d_object.h"
#include "Vector3.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include <vector>

#include "image.h"
#include "noise.h"
#include "refRenderer.h"
#include "sceneLoader.h"
#include "util.h"

RefRenderer::RefRenderer() { image = NULL; }

RefRenderer::~RefRenderer() {
  if (image) {
    delete image;
  }
}

const Image* RefRenderer::getImage() { return image; }

void RefRenderer::setup() {
  // nothing to do here
}

// allocOutputImage --
//
// Allocate buffer the renderer will render into.  Check status of
// image first to avoid memory leak.
void RefRenderer::allocOutputImage(int width, int height) {
  if (image) delete image;
  image = new Image(width, height);
}

// clearImage --
//
// Clear's the renderer's target image.  The state of the image after
// the clear depends on the scene being rendered.
void RefRenderer::clearImage() { image->clear(1.f, 1.f, 1.f, 1.f); }

void RefRenderer::loadScene(SceneName name) {
  printf("Loading scene\n");
  this->sceneName = name;
  scene = ::loadScene(name, image->width, image->height);
  scene->serialize(numTriangles, vertices, colors);
  printf("numTriangles: %d\n", numTriangles);
}

// advanceAnimation --
//
// Advance the simulation one time step. Move the camera position
void RefRenderer::advanceAnimation() {
  // TODO: move camera
  scene->cameraRotator.rotateCamera(scene->camera);
}

static float getTriangleZ(float px, float py, float* projectedVertices) {
  float x1 = projectedVertices[0];
  float y1 = projectedVertices[1];
  float x2 = projectedVertices[3];
  float y2 = projectedVertices[4];
  float x3 = projectedVertices[6];
  float y3 = projectedVertices[7];
  float lambda1 = ((y2 - y3) * (px - x3) + (x3 - x2) * (py - y3)) /
                  ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
  float lambda2 = ((y3 - y1) * (px - x3) + (x1 - x3) * (py - y3)) /
                  ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
  float lambda3 = 1 - lambda1 - lambda2;
  if (lambda1 >= 0 && lambda2 >= 0 && lambda3 >= 0 && lambda1 <= 1 &&
      lambda2 <= 1 && lambda3 <= 1) {
    float result = lambda1 * projectedVertices[2] +
                   lambda2 * projectedVertices[5] +
                   lambda3 * projectedVertices[8];
    return result;
  } else {
    return -1;
  }
}

static void rasterization(int numTriangles, float* projectedVertices,
                          const float* vertices, const float* colors,
                          float* outColor, float x, float y) {
  float minZ = std::numeric_limits<float>::max();
  for (int i = 0; i < numTriangles; i++) {
    float z = getTriangleZ(x, y, projectedVertices + i * 9);
    if (z < 0) {
      // not in triangle
      continue;
    }
    if (z < minZ) {
      minZ = z;
      outColor[0] = colors[i * 4];
      outColor[1] = colors[i * 4 + 1];
      outColor[2] = colors[i * 4 + 2];
      outColor[3] = colors[i * 4 + 3];
    }
  }
}

// Triangle* RefRenderer::initializePolygons() {
//   Triangle* triangles = new Triangle[numTriangles];

//   for (int polygonIndex = 0; polygonIndex < numTriangles; ++polygonIndex) {
//     std::array<Vector3, 3> polygonVertices;

//     for (int i = 0; i < 9; i += 3) {
//       polygonVertices[i] =
//           Vector3(vertices[i], vertices[i + 1], vertices[i + 2]);
//     }

//     std::array<float, 4> polygonColors = {
//         colors[polygonIndex], colors[polygonIndex + 1],
//         colors[polygonIndex + 2], colors[polygonIndex + 3]};

//     triangles[polygonIndex] = Triangle(polygonVertices, polygonColors);
//   }

//   return triangles;
// }

static void calClipSpaceVertex(float result[4], float combinedMatrix[4][4],
                               float vec4[4]) {
  for (int i = 0; i < 4; ++i) {
    result[i] = combinedMatrix[i][0] * vec4[0] +
                combinedMatrix[i][1] * vec4[1] +
                combinedMatrix[i][2] * vec4[2] + combinedMatrix[i][3] * vec4[3];
  }
}

Vector3 RefRenderer::transformVertex(const Vector3& vertex,
                                     float combinedMatrix[4][4]) {
  float vec4[4] = {vertex.x, vertex.y, vertex.z, 1.0f};

  float clipSpaceVertex[4];
  calClipSpaceVertex(clipSpaceVertex, combinedMatrix, vec4);

  Vector3 vec3{clipSpaceVertex[0] / clipSpaceVertex[3],
               clipSpaceVertex[1] / clipSpaceVertex[3],
               clipSpaceVertex[2] / clipSpaceVertex[3]};

  return Vector3((vec3.x + 1.0) / 2.0, (vec3.y + 1.0) / 2.0, vec3.z);
}

void RefRenderer::render() {
  float combinedMatrix[4][4];
  scene->camera.calculateViewMatrix(combinedMatrix);
  // printf("Combined Matrix: \n");
  // for (int i = 0; i < 4; ++i) {
  //   printf("%f, %f, %f, %f\n", combinedMatrix[i][0], combinedMatrix[i][1],
  //          combinedMatrix[i][2], combinedMatrix[i][3]);
  // }

  float* projectedVertices = new float[numTriangles * 3 * 3];

  printf("Rendering...\n");

  // printf("original vertices: \n");
  // for (int i = 0; i < numTriangles; ++i) {
  //   for (int j = 0; j < 3; ++j) {
  //     printf("%f %f %f\n", vertices[i * 9 + j * 3], vertices[i * 9 + j * 3 +
  //     1],
  //            vertices[i * 9 + j * 3 + 2]);
  //   }
  // }

  for (int i = 0; i < numTriangles; ++i) {
    for (int j = 0; j < 3; ++j) {
      Vector3 vec = transformVertex(
          Vector3(vertices[i * 9 + j * 3], vertices[i * 9 + j * 3 + 1],
                  vertices[i * 9 + j * 3 + 2]),
          combinedMatrix);
      projectedVertices[i * 9 + j * 3] = vec.x;
      projectedVertices[i * 9 + j * 3 + 1] = vec.y;
      projectedVertices[i * 9 + j * 3 + 2] = vec.z;
    }
  }
  // printf("projected vertices: \n");
  // for (int i = 0; i < numTriangles * 9; ++i) {
  //   if (i != 0 && i % 9 == 0) {
  //     printf("\n");
  //   }
  //   printf("%f ", projectedVertices[i]);
  // }
  // printf("\n");

  printf("Rasterizing...\n");

  for (int x = 0; x < image->width; x++) {
    for (int y = 0; y < image->height; y++) {
      float px = (x + 0.5f) / image->width;
      float py = (y + 0.5f) / image->height;
      rasterization(numTriangles, projectedVertices, vertices, colors,
                    &image->data[4 * (y * image->width + x)], px, py);
    }
  }
}
