/**
 * @file scene.cpp
 *
 * @author Sean James
 */

#include <scene.h>

Scene::Scene(Camera *camera, std::shared_ptr<Image> output,
    std::shared_ptr<Sampler> environment_sampler,
    std::shared_ptr<Image> environment)
    : camera(camera),
      output(output),
      environment_sampler(environment_sampler),
      environment(environment),
      poly_id(0)
{
}

void Scene::addPoly(Triangle poly, Shader *shader) {
    poly.triangle_id = poly_id++;
    triangles.push_back(poly);
    shaderMap.push_back(shader);
}

void Scene::addLight(Light *light) {
    lights.push_back(light);
}
