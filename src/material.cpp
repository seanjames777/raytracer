/**
 * @file material.cpp
 *
 * @author Sean James
 */

#include <material.h>
#include <raytracer.h>

Material::Material(vec3 ambient, vec3 diffuse, vec3 specular, float specularPower,
    float reflection, float refraction, float ior)
    : ambient(ambient),
      diffuse(diffuse),
      specular(specular),
      specularPower(specularPower),
      reflection(reflection),
      refraction(refraction),
      ior(ior)
{
}

vec3 Material::shade(Ray ray, Collision *result, Scene *scene, Raytracer *raytracer, int depth) {
    CollisionEx resultEx;
    scene->polys[result->polygonID].getCollisionEx(ray, result, &resultEx, true);

    //return resultEx.normal;

    vec3 color = vec3(0, 0, 0);
    color += ambient;

    vec3 reflection = vec3(0, 0, 0);

    if (this->reflection > 0.0f)
        reflection = raytracer->getEnvironmentReflection(result, &resultEx) * this->reflection;

    vec3 refraction = vec3(0, 0, 0);

    if (this->refraction > 0.0f)
        refraction = raytracer->getEnvironmentRefraction(result, ior, &resultEx) * this->refraction;

    float schlick_amt = 1.0f;

    if (this->reflection > 0.0f && this->refraction > 0.0f)
        schlick_amt = schlick(-resultEx.ray.direction, resultEx.normal, 1.0f, ior);

    color += refraction * (1.0f - schlick_amt) + reflection * schlick_amt;

    //color += raytracer->getIndirectLighting(result, &resultEx, depth);

    vec3 geomDiffuse = vec3(resultEx.color.x, resultEx.color.y, resultEx.color.z);

    for (auto it = scene->lights.begin(); it != scene->lights.end(); it++) {
        Light *light = *it;
        float shadow = raytracer->getShadow(result, &resultEx, light) * .8f + .2f;

        vec3 lcolor =  light->getColor(resultEx.position);
        vec3  ldir  = -light->getDirection(resultEx.position);

        float ndotl =  SATURATE(dot(resultEx.normal, ldir));

        vec3  view  =  resultEx.ray.direction;
        vec3  ref   =  reflect(ldir, resultEx.normal);

        float rdotv =  SATURATE(dot(ref, view));
        float specf =  powf(rdotv, specularPower);
        vec3 spec  =  specular * specf;

        color += (lcolor * (diffuse * geomDiffuse * ndotl) + spec) * shadow;
    }

    float occlusion = raytracer->getAmbientOcclusion(result, &resultEx);
    color = color * occlusion;

    return color;
}
