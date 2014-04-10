/**
 * @file material.cpp
 *
 * @author Sean James
 */

#include <material.h>
#include <raytracer.h>

Material::Material(Vec3 ambient, Vec3 diffuse, Vec3 specular, float specularPower,
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

Vec3 Material::shade(CollisionResult *result, Scene *scene, Raytracer *raytracer, int depth) {
	Vec3 color = Vec3(0, 0, 0);
	//color += ambient;

	Vec3 reflection = Vec3(0, 0, 0);

	if (this->reflection > 0.0f)
		reflection = raytracer->getEnvironmentReflection(result) * this->reflection;

	Vec3 refraction = Vec3(0, 0, 0);

	if (this->refraction > 0.0f)
		refraction = raytracer->getEnvironmentRefraction(result, ior) * this->refraction;

	float schlick = 1.0f;

	if (this->reflection > 0.0f && this->refraction > 0.0f)
		schlick = Vec3::schlick(-result->ray.direction, result->normal, 1.0f, ior);

	//color += refraction * (1.0f - schlick) + reflection * schlick;

	color += raytracer->getIndirectLighting(result, depth);

	for (auto it = scene->lights.begin(); it != scene->lights.end(); it++) {
		Light *light = *it;
		float shadow = raytracer->getShadow(result, light);

		Vec3 lcolor =  light->getColor(result->position);
		Vec3  ldir  = -light->getDirection(result->position);

		float ndotl =  SATURATE(result->normal.dot(ldir));

		Vec3  view  =  result->ray.direction;
		Vec3  ref   =  ldir.reflect(result->normal);

		float rdotv =  SATURATE(ref.dot(view));
		float specf =  powf(rdotv, specularPower);
		Vec3 spec  =  specular * specf;

		Vec3 geomDiffuse = Vec3(result->color.x, result->color.y, result->color.z);

		color += (lcolor * (diffuse * geomDiffuse * ndotl) + spec) * shadow;
	}

	float occlusion = raytracer->getAmbientOcclusion(result) * .5f + .5f;
	color = color * occlusion;
	//color = Vec3(occlusion, occlusion, occlusion);

	return color;
}
