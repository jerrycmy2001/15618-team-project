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
}

static inline void lookupColor(float coord, float& r, float& g, float& b) {
  const int N = 5;

  float lookupTable[N][3] = {
      {1.f, 1.f, 1.f}, {1.f, 1.f, 1.f},  {.8f, .9f, 1.f},
      {.8f, .9f, 1.f}, {.8f, 0.8f, 1.f},
  };

  float scaledCoord = coord * (N - 1);

  int base = std::min(static_cast<int>(scaledCoord), N - 1);

  // linearly interpolate between values in the table based on the
  // value of coord
  float weight = scaledCoord - static_cast<float>(base);
  float oneMinusWeight = 1.f - weight;

  r = (oneMinusWeight * lookupTable[base][0]) +
      (weight * lookupTable[base + 1][0]);
  g = (oneMinusWeight * lookupTable[base][1]) +
      (weight * lookupTable[base + 1][1]);
  b = (oneMinusWeight * lookupTable[base][2]) +
      (weight * lookupTable[base + 1][2]);
}

float edgeFunction(float px, float py, float x1, float y1, float x2, float y2) {
  return (py - y1) * (x2 - x1) - (px - x1) * (y2 - y1);
}

float getTriangleZ(float px, float py, float* projectedVertices) {
  float e1 = edgeFunction(px, py, projectedVertices[0], projectedVertices[1],
                          projectedVertices[3], projectedVertices[4]);
  float e2 = edgeFunction(px, py, projectedVertices[3], projectedVertices[4],
                          projectedVertices[6], projectedVertices[7]);
  float e3 = edgeFunction(px, py, projectedVertices[6], projectedVertices[7],
                          projectedVertices[0], projectedVertices[1]);
  if ((e1 >= 0 && e2 >= 0 && e3 >= 0) || (e1 <= 0 && e2 <= 0 && e3 <= 0)) {
    float result = abs(e1 * projectedVertices[8] + e2 * projectedVertices[2] +
                       e3 * projectedVertices[5]);
    // printf(
    //     "px = %f, py = %f, e1 = %f, e2 = %f, e3 = %f, projectedVertices = %f
    //     "
    //     "%f %f, result = %f\n",
    //     px, py, e1, e2, e3, projectedVertices[8], projectedVertices[5],
    //     projectedVertices[2], result);
    return result;
  } else {
    return -1;
  }
}

void rasterization(int numTriangles, float* projectedVertices,
                   const float* vertices, const float* colors, float* outColor,
                   float x, float y) {
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

void calClipSpaceVertex(float result[4], float combinedMatrix[4][4],
                        float vec4[4]) {
  for (int i = 0; i < 4; ++i) {
    result[i] = combinedMatrix[i][0] * vec4[0] +
                combinedMatrix[i][1] * vec4[1] +
                combinedMatrix[i][2] * vec4[2] + combinedMatrix[i][3] * vec4[3];
  }
}

Vector3 RefRenderer::transformVertex(const Vector3& vertex) {
  float combinedMatrix[4][4];
  scene->camera.calculateViewMatrix(combinedMatrix);

  printf("Combined Matrix: \n");

  for (int i = 0; i < 4; ++i) {
    printf("%f, %f, %f, %f\n", combinedMatrix[i][0], combinedMatrix[i][1],
           combinedMatrix[i][2], combinedMatrix[i][3]);
  }

  float vec4[4] = {vertex.x, vertex.y, vertex.z, 1.0f};

  float clipSpaceVertex[4];
  calClipSpaceVertex(clipSpaceVertex, combinedMatrix, vec4);

  Vector3 vec3{clipSpaceVertex[0] / clipSpaceVertex[3],
               clipSpaceVertex[1] / clipSpaceVertex[3],
               clipSpaceVertex[2] / clipSpaceVertex[3]};

  return Vector3((vec3.x + 1.0) / 2.0, (vec3.y + 1.0) / 2.0, vec3.z);
}

void RefRenderer::render() {
  float* projectedVertices = new float[numTriangles * 3 * 3];

  printf("Rendering...\n");

  for (int i = 0; i < numTriangles; ++i) {
    for (int j = 0; j < 3; ++j) {
      Vector3 vec = transformVertex(Vector3(vertices[i * 9 + j * 3],
                                            vertices[i * 9 + j * 3 + 1],
                                            vertices[i * 9 + j * 3 + 2]));
      projectedVertices[i * 9 + j * 3] = vec.x;
      projectedVertices[i * 9 + j * 3 + 1] = vec.y;
      projectedVertices[i * 9 + j * 3 + 2] = vec.z;
    }
  }
  for (int i = 0; i < numTriangles * 9; ++i) {
    if (i != 0 && i % 9 == 0) {
      printf("\n");
    }
    printf("%f ", projectedVertices[i]);
  }
  printf("\n");

  printf("Rasterizing...\n");

  for (int x = 0; x < image->width; x++) {
    for (int y = 0; y < image->height; y++) {
      float px = (x + 0.5f) / image->width;
      float py = (y + 0.5f) / image->height;
      rasterization(numTriangles, projectedVertices, vertices, colors,
                    &image->data[4 * (y * image->width + x)], px, py);
    }
  }
  // render all circles
  // for (int circleIndex = 0; circleIndex < numberOfCircles; circleIndex++) {
  //   int index3 = 3 * circleIndex;

  //   float px = position[index3];
  //   float py = position[index3 + 1];
  //   float pz = position[index3 + 2];
  //   float rad = radius[circleIndex];

  //   // compute the bounding box of the circle.  This bounding box
  //   // is in normalized coordinates
  //   float minX = px - rad;
  //   float maxX = px + rad;
  //   float minY = py - rad;
  //   float maxY = py + rad;

  //   // convert normalized coordinate bounds to integer screen
  //   // pixel bounds.  Clamp to the edges of the screen.
  //   int screenMinX =
  //       CLAMP(static_cast<int>(minX * image->width), 0, image->width);
  //   int screenMaxX =
  //       CLAMP(static_cast<int>(maxX * image->width) + 1, 0, image->width);
  //   int screenMinY =
  //       CLAMP(static_cast<int>(minY * image->height), 0, image->height);
  //   int screenMaxY =
  //       CLAMP(static_cast<int>(maxY * image->height) + 1, 0, image->height);

  //   float invWidth = 1.f / image->width;
  //   float invHeight = 1.f / image->height;

  //   // for each pixel in the bounding box, determine the circle's
  //   // contribution to the pixel.  The contribution is computed in
  //   // the function shadePixel.  Since the circle does not fill
  //   // the bounding box entirely, not every pixel in the box will
  //   // receive contribution.
  //   for (int pixelY = screenMinY; pixelY < screenMaxY; pixelY++) {
  //     // pointer to pixel data
  //     float* imgPtr = &image->data[4 * (pixelY * image->width + screenMinX)];

  //     for (int pixelX = screenMinX; pixelX < screenMaxX; pixelX++) {
  //       // When "shading" the pixel ("shading" = computing the
  //       // circle's color and opacity at the pixel), we treat
  //       // the pixel as a point at the center of the pixel.
  //       // We'll compute the color of the circle at this
  //       // point.  Note that shading math will occur in the
  //       // normalized [0,1]^2 coordinate space, so we convert
  //       // the pixel center into this coordinate space prior
  //       // to calling shadePixel.
  //       float pixelCenterNormX = invWidth * (static_cast<float>(pixelX) +
  //       0.5f); float pixelCenterNormY =
  //           invHeight * (static_cast<float>(pixelY) + 0.5f);
  //       shadePixel(pixelCenterNormX, pixelCenterNormY, px, py, pz, imgPtr,
  //                  circleIndex);
  //       imgPtr += 4;
  //     }
  //   }
  // }
}
