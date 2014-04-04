/**
 * @file pointlight.cpp
 *
 * @author Sean James
 */

#include "pointlight.h"

PointLight::PointLight(Vec3 position, Vec3 color, float radius, float range, float power,
	bool shadow)
	: position(position), 
	  color(color),
	  radius(radius),
	  range2(range * range), 
	  power(power), 
	  shadow(shadow)
{
}

Vec3 PointLight::getDirection(Vec3 pos) {
	Vec3 dir = pos - position;
	dir.normalize();

	return dir;
}

Vec3 PointLight::getColor(Vec3 pos) {
	Vec3 diff = pos - position;
	float dist2 = diff.len2();

	float falloff = 1.0f - CLAMP(0.0f, 1.0f, dist2 / range2);
	falloff = powf(falloff, power);

	return color * falloff;
}

bool PointLight::castsShadows() {
	return shadow;
}

Vec3 PointLight::getShadowDir(Vec3 at) {
	Vec3 pos = this->position;

	if (radius > 0.0f)
		pos = randSphere(pos, radius);

	Vec3 dir = pos - at;
	dir.normalize();

	return dir;
}
