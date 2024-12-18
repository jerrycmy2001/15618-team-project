#include "3d_object.h"
#include "renderer.h"
#include "scene.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <vector>

#include "sceneLoader.h"
#include "util.h"

std::shared_ptr<Scene> loadScene(SceneName sceneName, int windowWidth,
                                 int windowHeight) {
  std::vector<Object3D*> objs;
  Camera cam;
  CameraRotator camRotator;
  float min_range = 0.0f;
  float max_range = 100.0f;
  float stride;
  std::vector<std::vector<Object3D*>> chars;
  switch (sceneName) {
    case SceneName::SINGLE_TRIANGLE:
      objs = {new Triangle(
          {Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0.5, sqrt(3) / 2.f, 0)},
          {1, 0, 0, 1})};
      cam = Camera({0.5, (float)sqrt(3) / 4.f, 2}, {0, 0, -1}, {0, 1, 0}, 90.0f,
                   1.0f, 10.0f, windowWidth, windowHeight);
      break;
    case SceneName::DOUBLE_OVERLAPPING_TRIANGLES:
      objs = {new Triangle({Vector3(0, 0, 0), Vector3(1, 0, 0),
                            Vector3(0.5, sqrt(3) / 2.f, 0)},
                           {1, 0, 0, 1}),
              new Triangle({Vector3(0.5, 0, -1), Vector3(1.5, 0, -1),
                            Vector3(1, sqrt(3) / 2.f, -1)},
                           {0, 1, 0, 1})};
      cam = Camera({0.5, (float)sqrt(3) / 4.f, 4}, {0, 0, -1}, {0, 1, 0}, 90.0f,
                   1.0f, 10.0f, windowWidth, windowHeight);
      camRotator =
          CameraRotator({0.5, (float)sqrt(3) / 4.f, 0}, {0, 1, 0}, 0.1f);
      break;
    case SceneName::NON_ORTHOGONAL_TRIANGLES:
      objs = {
          new Triangle({Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)},
                       {1, 0, 0, 1}),
          new Triangle({Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 0, 1)},
                       {0, 1, 0, 1}),
          new Triangle({Vector3(0, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0)},
                       {0, 0, 1, 1})};
      cam = Camera({3, 3, 3}, {-1, -1, -1}, {0, 0, 1}, 90.0f, 1.0f, 10.0f,
                   windowWidth, windowHeight);
      camRotator = CameraRotator({0, 0, 0}, {0, 0, 1}, 0.04f);
      break;
    case SceneName::SQUARE:
      objs = {new Square({Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)},
                         {1, 0, 0, 1})};
      cam = Camera({0.5, 0.5, 2}, {0, 0, -1}, {0, 0, 1}, 90.0f, 1.0f, 10.0f,
                   windowWidth, windowHeight);
      break;
    case SceneName::CUBE:
      objs = {new Cube({Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0),
                        Vector3(0, 0, 1)},
                       {{{1, 0, 0, 1},
                         {0, 1, 0, 1},
                         {0, 0, 1, 1},
                         {0, 0.5, 0.5, 1},
                         {0.5, 0, 0.5, 1},
                         {0.5, 0.5, 0, 1}}})};
      cam = Camera({2, 2, 2}, {-1, -1, -1}, {0, 0, 1}, 90.0f, 0.01f, 10.0f,
                   windowWidth, windowHeight);
      camRotator = CameraRotator({0.5, 0.5, 100}, {0, 0, 1}, 0.04f);
      break;
    case SceneName::TETRAHEDRON:
      objs = {
          new RegularTetrahedron(
              {
                  {Vector3(0, 0, 0), Vector3(1, 0, 0),
                   Vector3(0.5, std::sqrt(3) / 2.0, 0),
                   Vector3(0.5, std::sqrt(3) / 4.0, std::sqrt(2) / 2.f)},
              },
              {{{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}, {0.5, 0.5, 0.5, 1}}}),
          new RegularTetrahedron(
              Vector3(2, 0, 1), 1, Vector3(0, 0, -std::sqrt(2) / 2.f),
              Vector3(1, 0, 0),
              {{{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}, {0.5, 0.5, 0.5, 1}}}),
      };

      cam = Camera({2, 2, 1}, {-1, -1, -0.2}, {0, 0, 1}, 90.0f, 0.01f, 10.0f,
                   windowWidth, windowHeight);
      camRotator = CameraRotator({0, 0, 0}, {0, 0, 1}, 0.04f);
      break;
    case SceneName::RAND8:
    case SceneName::RAND27:
    case SceneName::RAND64:
    case SceneName::RAND125:
    case SceneName::RAND216:
    case SceneName::RAND343:
    case SceneName::RAND512:
    case SceneName::RAND729:
    case SceneName::RAND1000:
    case SceneName::RAND3375:
    case SceneName::RAND8000:
      if (sceneName == SceneName::RAND8) {
        stride = max_range / 2.0f;
      } else if (sceneName == SceneName::RAND27) {
        stride = max_range / 3.0f;
      } else if (sceneName == SceneName::RAND64) {
        stride = max_range / 4.0f;
      } else if (sceneName == SceneName::RAND125) {
        stride = max_range / 5.0f;
      } else if (sceneName == SceneName::RAND216) {
        stride = max_range / 6.0f;
      } else if (sceneName == SceneName::RAND343) {
        stride = max_range / 7.0f;
      } else if (sceneName == SceneName::RAND512) {
        stride = max_range / 8.0f;
      } else if (sceneName == SceneName::RAND729) {
        stride = max_range / 9.0f;
      } else if (sceneName == SceneName::RAND1000) {
        stride = max_range / 10.0f;
      } else if (sceneName == SceneName::RAND3375) {
        stride = max_range / 15.0f;
      } else if (sceneName == SceneName::RAND8000) {
        stride = max_range / 20.0f;
      }
      for (float x = min_range; x < max_range; x += stride)
        for (float y = min_range; y < max_range; y += stride)
          for (float z = min_range; z < max_range; z += stride) {
            if (rand() % 2 == 0) {
              objs.push_back(new RegularTetrahedron(x, x + stride, y,
                                                    y + stride, z, z + stride));
            } else {
              objs.push_back(
                  new Cube(x, x + stride, y, y + stride, z, z + stride));
            }
          }

      // cam = Camera(min_range, max_range, windowWidth, windowHeight);
      // camRotator = CameraRotator(min_range, max_range);
      cam = Camera({-100, 0, 50}, {1, 0, 0}, {0, 0, 1}, 90.0f, 0.01f, 100000.0f,
                   windowWidth, windowHeight);
      camRotator = CameraRotator({50, 50, 50}, {0, 0, 1}, 0.04f);
      break;
    case SceneName::COURSE_TITLE:
      chars = {{
                   new RegularTetrahedron(
                       {
                           {Vector3(-9, 0, 3.7), Vector3(-9, 1, 3.7),
                            Vector3(-8, 1, 3.7), Vector3(-8.5, 0.5, 4.2)},
                       },
                       {{{0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(-8, 0, 3.7), Vector3(-9, 0, 3.7),
                            Vector3(-8, 1, 3.7), Vector3(-8.5, 0.5, 4.2)},
                       },
                       {{{0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1}}}),
                   new Cube({Vector3(-9, 0, 3.7), Vector3(-9, 1, 3.7),
                             Vector3(-8, 0, 3.7), Vector3(-9, 0, -3.7)},
                            {{{0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(-9, 0, -3.7), Vector3(-9, 1, -3.7),
                            Vector3(-8, 1, -3.7), Vector3(-8.5, 0.5, -4.2)},
                       },
                       {{{0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(-8, 0, -3.7), Vector3(-9, 0, -3.7),
                            Vector3(-8, 1, -3.7), Vector3(-8.5, 0.5, -4.2)},
                       },
                       {{{0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1},
                         {0.239, 0.333, 0.808, 1}}}),
               },
               {
                   new Cube({Vector3(-6, 0, 3.2), Vector3(-6, 1, 3.2),
                             Vector3(-6, 0, 4), Vector3(-2, 0, 3.2)},
                            {{{0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1}}}),
                   new Cube({Vector3(-6, 0, 3.2), Vector3(-6, 1, 3.2),
                             Vector3(-5, 0, 3.2), Vector3(-6, 0, -0.4)},
                            {{{0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1}}}),
                   new Cube({Vector3(-5, 0, -0.4), Vector3(-5, 1, -0.4),
                             Vector3(-5, 0, 0.4), Vector3(-2, 0, -0.4)},
                            {{{0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1}}}),
                   new Cube({Vector3(-3, 0, -0.4), Vector3(-3, 1, -0.4),
                             Vector3(-3, 0, -3.2), Vector3(-2, 0, -0.4)},
                            {{{0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1}}}),
                   new Cube({Vector3(-6, 0, -4), Vector3(-6, 1, -4),
                             Vector3(-6, 0, -3.2), Vector3(-2, 0, -4)},
                            {{{0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1}}}),
               },
               {
                   new Cube({Vector3(0, 0, 0), Vector3(0, 1, 0),
                             Vector3(1, 0, 0), Vector3(2, 0, 4)},
                            {{{0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1},
                              {0.325, 0.522, 0.859, 1}}}),
                   new Cube({Vector3(0, 0, 0), Vector3(0, 1, 0),
                             Vector3(1, 0, 0), Vector3(2, 0, -4)},
                            {{{0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1}}}),
                   new Cube({Vector3(1, 0, 0), Vector3(1, 1, 0),
                             Vector3(1.4, 0, -1), Vector3(4.8, 0, 0)},
                            {{{0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1}}}),
                   new Cube({Vector3(4.5, 0, -1), Vector3(4.5, 1, -1),
                             Vector3(5.2, 0, -1), Vector3(3, 0, -4)},
                            {{{0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1},
                              {0.208, 0.506, 0.835, 1}}}),
               },
               {
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, 3), Vector3(8, 0.5, 3),
                            Vector3(7, 0.5, 3), Vector3(7.5, 0.5, 4.1)},
                       },
                       {{{0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 1, 3), Vector3(8, 0.5, 3),
                            Vector3(7, 0.5, 3), Vector3(7.5, 0.5, 4.1)},
                       },
                       {{{0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, 3), Vector3(8, 0.5, 3),
                            Vector3(7, 0.5, 3), Vector3(7.5, 0.5, 2)},
                       },
                       {{{0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 1, 3), Vector3(8, 0.5, 3),
                            Vector3(7, 0.5, 3), Vector3(7.5, 0.5, 2)},
                       },
                       {{{0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, 1), Vector3(8, 0.5, 1),
                            Vector3(7, 0.5, 1), Vector3(7.5, 0.5, 2)},
                       },
                       {{{0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 1, 1), Vector3(8, 0.5, 1),
                            Vector3(7, 0.5, 1), Vector3(7.5, 0.5, 2)},
                       },
                       {{{0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, 1), Vector3(8, 0.5, 1),
                            Vector3(7, 0.5, 1), Vector3(7.5, 0.5, -0)},
                       },
                       {{{0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, 1), Vector3(8, 0.5, 1),
                            Vector3(7, 0.5, 1), Vector3(7.5, 0.5, -0)},
                       },
                       {{{0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1},
                         {0.208, 0.506, 0.835, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, -1), Vector3(8, 0.5, -1),
                            Vector3(7, 0.5, -1), Vector3(7.5, 0.5, 0)},
                       },
                       {{{0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 1, -1), Vector3(8, 0.5, -1),
                            Vector3(7, 0.5, -1), Vector3(7.5, 0.5, 0)},
                       },
                       {{{0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, -1), Vector3(8, 0.5, -1),
                            Vector3(7, 0.5, -1), Vector3(7.5, 0.5, -2)},
                       },
                       {{{0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 1, -1), Vector3(8, 0.5, -1),
                            Vector3(7, 0.5, -1), Vector3(7.5, 0.5, -2)},
                       },
                       {{{0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, -3), Vector3(8, 0.5, -3),
                            Vector3(7, 0.5, -3), Vector3(7.5, 0.5, -2)},
                       },
                       {{{0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 1, -3), Vector3(8, 0.5, -3),
                            Vector3(7, 0.5, -3), Vector3(7.5, 0.5, -2)},
                       },
                       {{{0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 0, -3), Vector3(8, 0.5, -3),
                            Vector3(7, 0.5, -3), Vector3(7.5, 0.5, -4.1)},
                       },
                       {{{0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1}}}),
                   new RegularTetrahedron(
                       {
                           {Vector3(7.5, 1, -3), Vector3(8, 0.5, -3),
                            Vector3(7, 0.5, -3), Vector3(7.5, 0.5, -4.1)},
                       },
                       {{{0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1},
                         {0.239, 0.404, 0.808, 1}}}),
               },
               {
                   new Cube({Vector3(10.7, 0, 0.4), Vector3(10.7, 1, 0.4),
                             Vector3(11.5, 0, 0.4), Vector3(10.7, 0, 3.2)},
                            {{{0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1}}}),
                   new Cube({Vector3(11.5, 0, 3.2), Vector3(11.5, 1, 3.2),
                             Vector3(11.5, 0, 4), Vector3(13.5, 0, 3.2)},
                            {{{0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1}}}),
                   new Cube({Vector3(13.5, 0, 0.4), Vector3(13.5, 1, 0.4),
                             Vector3(14.3, 0, 0.4), Vector3(13.5, 0, 3.2)},
                            {{{0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1}}}),
                   new Cube({Vector3(10.7, 0, -0.4), Vector3(10.7, 1, -0.4),
                             Vector3(11.5, 0, -0.4), Vector3(10.7, 0, -3.2)},
                            {{{0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1}}}),
                   new Cube({Vector3(13.5, 0, -0.4), Vector3(13.5, 1, -0.4),
                             Vector3(14.3, 0, -0.4), Vector3(13.5, 0, -3.2)},
                            {{{0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1}}}),
                   new Cube({Vector3(11.5, 0, -0.4), Vector3(11.5, 1, -0.4),
                             Vector3(11.5, 0, 0.4), Vector3(13.5, 0, -0.4)},
                            {{{0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1},
                              {0.239, 0.404, 0.808, 1}}}),
                   new Cube({Vector3(11.5, 0, -3.2), Vector3(11.5, 1, -3.2),
                             Vector3(11.5, 0, -4), Vector3(13.5, 0, -3)},
                            {{{0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1},
                              {0.239, 0.333, 0.808, 1}}}),
               }};
      for (auto v : chars) {
        for (auto obj : v) {
          objs.push_back(obj);
        }
      }
      cam = Camera({2, 17, 0}, {0, -1, 0}, {0, 0, 1}, 90.0f, 0.01f, 20.0f,
                   windowWidth, windowHeight);
      camRotator = CameraRotator({2, 0, 0}, {0, 0, 1}, 0.04f);
      break;
    case SceneName::VISUAL_ILLUSION:
      objs = {
          new Cube({Vector3(0.9, 0, 0), Vector3(1, 0, 0), Vector3(0.9, 0, 0.1),
                    Vector3(0.9, 1, 0)},
                   {{{0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1}}}),
          new Cube({Vector3(0, 0.9, 0), Vector3(0, 1, 0), Vector3(0, 0.9, 0.1),
                    Vector3(1, 0.9, 0)},
                   {{{0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1}}}),
          new Cube({Vector3(0.9, 0, 0), Vector3(1, 0, 0), Vector3(0.9, 0.1, 0),
                    Vector3(0.9, 0, 0.9)},
                   {{{0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1}}}),
          new Cube({Vector3(0, 0.9, 0), Vector3(0, 1, 0), Vector3(0.1, 0.9, 0),
                    Vector3(0, 0.9, 0.9)},
                   {{{0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1}}}),
          new Cube({Vector3(0, 0, 0.9), Vector3(0.1, 0, 0.9), Vector3(0, 0, 1),
                    Vector3(0, 1, 0.9)},
                   {{{0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1}}}),
          new Cube({Vector3(0, 0, 0.9), Vector3(0, 0.1, 0.9), Vector3(0, 0, 1),
                    Vector3(1, 0, 0.9)},
                   {{{0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1},
                     {0, 0, 1, 1}}}),
          // start lower
          new Cube({Vector3(0.9, 0, 0.9), Vector3(1, 0, 0.9),
                    Vector3(0.9, 0, 1), Vector3(0, 0.1, -0.7)},
                   {{{0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1}}}),
          new Cube({Vector3(0.9, 0.9, 0), Vector3(1, 0.9, 0),
                    Vector3(0.9, 1, 0), Vector3(0, 0, -0.6)},
                   {{{0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1}}}),
          new Cube({Vector3(0, 0.9, 0.9), Vector3(0, 0.9, 1),
                    Vector3(0, 1, 0.9), Vector3(0.1, 0, -0.7)},
                   {{{0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1},
                     {0, 0, 0.7, 1}}}),
          // start upper
          new Cube({Vector3(0.9, 0, 0), Vector3(0.9, 0.1, 0),
                    Vector3(0.9, 0, 0.1), Vector3(1.1, 1, 1.5)},
                   {{{0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1}}}),
          new Cube({Vector3(0, 0.9, 0), Vector3(0.1, 0.9, 0),
                    Vector3(0, 0.9, 0.1), Vector3(1, 1.1, 1.5)},
                   {{{0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1}}}),
          new Cube({Vector3(0, 0, 0.9), Vector3(0.1, 0, 0.9),
                    Vector3(0, 0.1, 0.9), Vector3(1, 1, 1.6)},
                   {{{0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1},
                     {0.3, 0.3, 1, 1}}}),
      };
      cam = Camera({3, 3, 4}, {-1, -1, -1.2}, {0, 0, 1}, 90.0f, 0.01f, 10.0f,
                   windowWidth, windowHeight);
      camRotator = CameraRotator({0.5, 0.5, 100}, {0, 0, 1}, 0.04f);
      break;
    default:
      fprintf(stderr, "Error: can't load scene (unknown scene)\n");
      exit(1);
  }
  std::vector<Triangle> triangles;
  for (const Object3D* obj : objs) {
    std::vector<Triangle> tmp = obj->getTriangles();
    triangles.insert(triangles.end(), tmp.begin(), tmp.end());
  }
  for (Object3D* obj : objs) {
    delete obj;
  }
  return std::make_shared<Scene>(triangles, cam, camRotator);
}
