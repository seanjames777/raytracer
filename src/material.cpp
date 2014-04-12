/**
 * @file material.cpp
 *
 * @author Sean James
 */

#include <material.h>
#include <raytracer.h>

static Vec3 node_colors[32];
static bool colors_init = false;

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
	if (!colors_init) {
		for (int i = 0; i < 32; i++)
			node_colors[i] = Vec3(randf(0.5f, 1.0f), randf(0.5f, 1.0f), randf(0.5f, 1.0f));
		colors_init = true;
	}
}

Vec3 Material::shade(Ray ray, Collision *result, Scene *scene, Raytracer *raytracer, int depth) {
	CollisionEx resultEx;
	scene->polys[result->polygonID].getCollisionEx(ray, result, &resultEx, true);

	Vec3 color = Vec3(0, 0, 0);
	//color += ambient;

	Vec3 reflection = Vec3(0, 0, 0);

	if (this->reflection > 0.0f)
		reflection = raytracer->getEnvironmentReflection(result, &resultEx) * this->reflection;

	Vec3 refraction = Vec3(0, 0, 0);

	if (this->refraction > 0.0f)
		refraction = raytracer->getEnvironmentRefraction(result, ior, &resultEx) * this->refraction;

	float schlick = 1.0f;

	if (this->reflection > 0.0f && this->refraction > 0.0f)
		schlick = Vec3::schlick(-resultEx.ray.direction, resultEx.normal, 1.0f, ior);

	//color += refraction * (1.0f - schlick) + reflection * schlick;

	//color += raytracer->getIndirectLighting(result, &resultEx, depth);

	for (auto it = scene->lights.begin(); it != scene->lights.end(); it++) {
		Light *light = *it;
		//float shadow = raytracer->getShadow(result, &resultEx, light);
		float shadow = 1.0f;

		Vec3 lcolor =  light->getColor(resultEx.position);
		Vec3  ldir  = -light->getDirection(resultEx.position);

		float ndotl =  SATURATE(resultEx.normal.dot(ldir));

		Vec3  view  =  resultEx.ray.direction;
		Vec3  ref   =  ldir.reflect(resultEx.normal);

		float rdotv =  SATURATE(ref.dot(view));
		float specf =  powf(rdotv, specularPower);
		Vec3 spec  =  specular * specf;

		Vec3 geomDiffuse = Vec3(resultEx.color.x, resultEx.color.y, resultEx.color.z);
		geomDiffuse = node_colors[result->KD_NODE % 32];

		color += (lcolor * (diffuse * geomDiffuse * ndotl) + spec) * shadow;
	}

	//float occlusion = raytracer->getAmbientOcclusion(result, &resultEx);
	//color = color * occlusion;
	//color = Vec3(occlusion, occlusion, occlusion);

	return color;
}
