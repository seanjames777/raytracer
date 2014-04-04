/**
 * @file scene.cpp
 *
 * @author Sean James
 */

#include <scene.h>

Scene::Scene(Camera *camera, Bitmap *output)
    : camera(camera),
      output(output)
{
}

void Scene::addShape(Shape *shape, Material *material) {
    shapes.push_back(shape);
    materialMap[shape] = material;
}

void Scene::addLight(Light *light) {
    lights.push_back(light);
}