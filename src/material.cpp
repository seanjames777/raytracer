/**
 * @file material.cpp
 *
 * @author Sean James
 */

#include <material.h>

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

Vec3 Material::shade(Light *light, CollisionResult *result) {
	Vec3 color =  light->getColor(result->position);
	Vec3  ldir  = -light->getDirection(result->position);

	float ndotl =  SATURATE(result->normal.dot(ldir));

	Vec3  view  =  result->ray.direction;
	Vec3  ref   =  ldir.reflect(result->normal);

	float rdotv =  SATURATE(ref.dot(view));
	float specf =  powf(rdotv, specularPower);
	Vec3 spec  =  specular * specf;

	Vec3 output = color * (diffuse * ndotl) + spec;

	return output;
}

Vec3 Material::getAmbient() {
	return ambient;
}

float Material::getIndexOfRefraction() {
	return ior;
}

float Material::getReflectionAmt() {
	return reflection;
}

float Material::getRefractionAmt() {
	return refraction;
}
