/**
 * @file shader/phongshader.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <shader/phongshader.h>

#include <core/raytracer.h>

PhongShader::PhongShader(
    const vec3      & ambient,
    const vec3      & diffuse,
    const vec3      & specular,
    float             specularPower,
    float             reflection,
    float             refraction,
    float             ior,
    Image<float, 3> * diffuseTexture,
    Sampler         * diffuseSampler)
    : ambient(ambient),
      diffuse(diffuse),
      specular(specular),
      specularPower(specularPower),
      reflection(reflection),
      refraction(refraction),
      ior(ior),
      diffuseSampler(diffuseSampler),
      diffuseTexture(diffuseTexture)
{
}

PhongShader::~PhongShader() {
}

vec3 PhongShader::shade(
    RayBuffer       & rayBuff,
    const Ray       & ray,
    const Collision & result,
    Scene           * scene,
    Raytracer       * raytracer) const
{
    /*Triangle *triangle = &scene->triangles[result->triangle_id];
    Vertex interp = triangle->interpolate(result->beta, result->gamma);

    vec3 color = vec3(0, 0, 0);
    color += ambient;

    vec3 reflection = vec3(0, 0, 0);

    if (this->reflection > 0.0f)
        reflection = raytracer->getEnvironmentReflection(ray.direction, interp.normal) * this->reflection;

    vec3 refraction = vec3(0, 0, 0);

    if (this->refraction > 0.0f)
        refraction = raytracer->getEnvironmentRefraction(ray.direction, interp.normal, ior) * this->refraction;

    float schlick_amt = 1.0f;

    if (this->reflection > 0.0f && this->refraction > 0.0f)
        schlick_amt = schlick(-ray.direction, interp.normal, 1.0f, ior);

    color += refraction * (1.0f - schlick_amt) + reflection * schlick_amt;

    //color += raytracer->getIndirectLighting(result, &resultEx, depth);

    vec3 geomDiffuse = vec3(interp.color.x, interp.color.y, interp.color.z);

    vec3 offset_origin = interp.position + triangle->normal * .001f;

    vec3 tex_diffuse;

    if (diffuse_texture != nullptr)
        diffuse_sampler->sample(diffuse_texture.get(), interp.uv).xyz();

    for (auto it = scene->lights.begin(); it != scene->lights.end(); it++) {
        Light *light = *it;
        float shadow = raytracer->getShadow(kdStack, offset_origin, light) * .8f + .2f;

        vec3 lcolor =  light->getColor(interp.position);
        vec3  ldir  = -light->getDirection(interp.position);

        float ndotl =  saturate(dot(interp.normal, ldir));

        vec3  view  =  ray.direction;
        vec3  ref   =  reflect(ldir, interp.normal);

        float rdotv =  saturate(dot(ref, view));
        float specf =  powf(rdotv, specularPower);
        vec3 spec  =  specular * specf;

        color += (lcolor * (diffuse * tex_diffuse * geomDiffuse * ndotl) + spec) * shadow;
    }

    float occlusion = raytracer->getAmbientOcclusion(kdStack, offset_origin, triangle->normal);
    color = color * occlusion;

    return color;*/

    return vec3(0, 0, 0);
}
