/**
 * @file scene.cpp
 *
 * @author Sean James
 */

#include <scene.h>

Scene::Scene(Camera *camera, Bitmap *output, Bitmap *environment)
    : camera(camera),
      output(output),
      environment(environment)
{
}

void Scene::addShape(Shape *shape, Material *material) {
    shapes.push_back(shape);
    materialMap[shape] = material;
}

void Scene::addLight(Light *light) {
    lights.push_back(light);
}