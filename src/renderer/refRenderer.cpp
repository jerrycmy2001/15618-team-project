#include <algorithm>
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

void RefRenderer::loadScene(SceneName scene) {
  sceneName = scene;
  loadCircleScene(sceneName, numPolygons, numVertices, vertices, endIndices,
                  colors);
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

// // shadePixel --
// //
// // Computes the contribution of the specified circle to the
// // given pixel.  All values are provided in normalized space, where
// // the screen spans [0,2]^2.  The color/opacity of the circle is
// // computed at the pixel center.
// void RefRenderer::shadePixel(float pixelCenterX, float pixelCenterY, float
// px,
//                              float py, float pz, float* pixelData,
//                              int circleIndex) {
//   float diffX = px - pixelCenterX;
//   float diffY = py - pixelCenterY;
//   float pixelDist = diffX * diffX + diffY * diffY;

//   float rad = radius[circleIndex];
//   float maxDist = rad * rad;

//   // circle does not contribute to the image
//   if (pixelDist > maxDist) return;

//   float colR, colG, colB;
//   float alpha;

//   // there is a non-zero contribution.  Now compute the shading
//   if (sceneName == SNOWFLAKES || sceneName == SNOWFLAKES_SINGLE_FRAME) {
//     // Snowflake opacity falls off with distance from center.
//     // Snowflake color is determined by distance from center and
//     // radially symmetric.  The color value f(dist) is looked up
//     // from a table.

//     const float kCircleMaxAlpha = .5f;
//     const float falloffScale = 4.f;

//     float normPixelDist = sqrt(pixelDist) / rad;
//     lookupColor(normPixelDist, colR, colG, colB);

//     float maxAlpha = kCircleMaxAlpha * CLAMP(.6f + .4f * (1.f - pz),
//     0.f, 1.f); alpha = maxAlpha * exp(-1.f * falloffScale * normPixelDist *
//     normPixelDist);

//   } else {
//     // simple: each circle has an assigned color
//     int index3 = 3 * circleIndex;
//     colR = color[index3];
//     colG = color[index3 + 1];
//     colB = color[index3 + 2];
//     alpha = .5f;
//   }

//   // The following code is *very important*: it blends the
//   // contribution of the circle primitive with the current state
//   // of the output image pixel.  This is a read-modify-write
//   // operation on the image, and it needs to be atomic.  Moreover,
//   // (and even more challenging) all writes to this pixel must be
//   // performed in same order as when the circles are processed
//   // serially.
//   //
//   // That is, if circle 1 and circle 2 both write to pixel P.
//   // circle 1's contribution *must* be blended in first, then
//   // circle 2's.  If this invariant is not preserved, the
//   // rendering of transparent circles will not be correct.

//   float oneMinusAlpha = 1.f - alpha;
//   pixelData[0] = alpha * colR + oneMinusAlpha * pixelData[0];
//   pixelData[1] = alpha * colG + oneMinusAlpha * pixelData[1];
//   pixelData[2] = alpha * colB + oneMinusAlpha * pixelData[2];
//   pixelData[3] += alpha;
// }

void RefRenderer::render() {
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
