#include "3d_object.h"
#include "renderer.h"
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
  switch (sceneName) {
    case SceneName::SINGLE_TRIANGLE:
      objs = {new Triangle(
          {Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0.5, sqrt(3) / 2.f, 0)},
          {255, 0, 0, 1})};
      cam = Camera({0.5, (float)sqrt(3) / 4.f, 2}, {0, 0, -1}, {0, 1, 0}, 90.0f,
                   1.0f, 10.0f, windowWidth, windowHeight);
      break;
    case SceneName::DOUBLE_OVERLAPPING_TRIANGLES:
      objs = {new Triangle({Vector3(0, 0, 0), Vector3(1, 0, 0),
                            Vector3(0.5, sqrt(3) / 2.f, 0)},
                           {255, 0, 0, 1}),
              new Triangle({Vector3(0.5, 0, -1), Vector3(1.5, 0, -1),
                            Vector3(1, sqrt(3) / 2.f, -1)},
                           {0, 255, 0, 1})};
      cam = Camera({0.5, (float)sqrt(3) / 4.f, 4}, {0, 0, -1}, {0, 1, 0}, 90.0f,
                   1.0f, 10.0f, windowWidth, windowHeight);
      camRotator =
          CameraRotator({0.5, (float)sqrt(3) / 4.f, 0}, {0, 1, 0}, 0.1f);
      break;
    case SceneName::NON_ORTHOGONAL_TRIANGLES:
      objs = {new Triangle({Vector3(0, 0, 0), Vector3(1, 0, 0),
                            Vector3(0.5, sqrt(3) / 2.f, 0)},
                           {255, 0, 0, 1}),
              new Triangle({Vector3(0.5, 0, -1), Vector3(1.5, 0, -1),
                            Vector3(1, sqrt(3) / 2.f, -1)},
                           {0, 255, 0, 1})};
      cam = Camera({0.5, (float)sqrt(3) / 4.f, 2}, {0.2, 0.2, -1}, {0, 1, 0},
                   90.0f, 1.0f, 10.0f, windowWidth, windowHeight);
      break;
    case SceneName::SQUARE:
      objs = {new Square({Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)},
                         {255, 0, 0, 1})};
      cam = Camera({0.5, 0.5, 2}, {0, 0, -1}, {0, 1, 0}, 90.0f, 1.0f, 10.0f,
                   windowWidth, windowHeight);
      break;
    case SceneName::CUBE:
      objs = {new Cube({Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0),
                        Vector3(0, 1, 1)},
                       {{{255, 0, 0, 1},
                         {0, 255, 0, 1},
                         {0, 0, 255, 1},
                         {255, 0, 0, 1},
                         {0, 255, 0, 1},
                         {0, 0, 255, 1}}})};
      cam = Camera({2, 2, 2}, {-1, -1, -1}, {0, 0, 1}, 90.0f, 1.0f, 10.0f,
                   windowWidth, windowHeight);
      break;
    case SceneName::TETRAHEDRON:
      objs = {
          new RegularTetrahedron(
              {
                  {Vector3(0, 0, 0), Vector3(1, 0, 0),
                   Vector3(0.5, std::sqrt(3) / 2.0, 0),
                   Vector3(0.5, std::sqrt(3) / 2.0, std::sqrt(2))},
              },
              {{{255, 0, 0, 1},
                {0, 255, 0, 1},
                {0, 0, 255, 1},
                {50, 50, 155, 1}}}),
          // new RegularTetrahedron(Vector3(2, 2, 2), Vector3(0, 0, -2),
          //                        Vector3(1, 0, 0),
          //                        {{{255, 0, 0, 1},
          //                          {0, 255, 0, 1},
          //                          {0, 0, 255, 1},
          //                          {0, 0, 255, 1}}}),
      };
      cam = Camera({4, 4, 2}, {-1, -1, -1}, {0, 0, 1}, 90.0f, 1.0f, 10.0f,
                   windowWidth, windowHeight);
      camRotator =
          CameraRotator({0.5, (float)sqrt(3) / 4.f, 0}, {0, 1, 0}, 0.1f);
      break;
    case SceneName::RAND:
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
