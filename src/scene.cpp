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

void Scene::addPoly(Polygon *poly, Material *material) {
    polys.push_back(poly);
    materialMap[poly] = material;
}

void Scene::addLight(Light *light) {
    lights.push_back(light);
}