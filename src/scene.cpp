/**
 * @file scene.cpp
 *
 * @author Sean James
 */

#include <scene.h>

Scene::Scene(Camera *camera, Image *output, Image *environment)
    : camera(camera),
      output(output),
      environment(environment),
      poly_id(0)
{
}

void Scene::addPoly(Triangle poly, Material *material) {
    poly.triangle_id = poly_id++;
    triangles.push_back(poly);
    materialMap.push_back(material);
}

void Scene::addLight(Light *light) {
    lights.push_back(light);
}
