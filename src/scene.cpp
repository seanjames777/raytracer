/**
 * @file scene.cpp
 *
 * @author Sean James
 */

#include <scene.h>

Scene::Scene(Camera *camera, Image *output, Image *environment)
    : camera(camera),
      output(output),
      environment(environment)
{
}

void Scene::addPoly(Polygon poly, Material *material) {
    polys.push_back(poly);
    materialMap.push_back(material);
    polyAccels.push_back(PolygonAccel(poly.v1.position, poly.v2.position, poly.v3.position,
        (unsigned int)polys.size() - 1));
}

void Scene::addLight(Light *light) {
    lights.push_back(light);
}