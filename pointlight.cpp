/*
 * Sean James
 *
 * pointlight.cpp
 *
 * Point light with an origin, radius, and falloff curve
 *
 */

#include "pointlight.h"

/*
 * Constructor accepts light parameters
 */
PointLight::PointLight(const Vec3 & Position, const Color & Color, float Radius, float Power, bool Shadow)
	: position(Position), 
	  color(Color), 
	  radius2(Radius * Radius), 
	  power(Power), 
	  shadow(Shadow)
{
}

/*
 * Get the direction of the point light at a given point
 */
Vec3 PointLight::getDirection(const Vec3 & pos) {
	Vec3 p = pos; // TODO
	Vec3 dir = p - position;
	dir.normalize();

	return dir;
}

/*
 * Get the color of the point light at a given point
 */
Color PointLight::getColor(const Vec3 & pos) {
	Vec3 p = pos; // TODO
	Vec3 diff = p - position;
	float dist2 = diff.len2();

	float falloff = 1.0f - CLAMP(0.0f, 1.0f, dist2 / radius2);
	falloff = pow(falloff, power);

	return color * falloff;
}

/*
 * Whether this point light casts shadows
 */
bool PointLight::castsShadows() {
	return shadow;
}

/*
 * Get the direction of a shadow-testing ray at a given
 * point, randomly jittered if soft-shadows are enabled
 */
Vec3 PointLight::getShadowDir(const Vec3 & at, int numShadowRays) {
	Vec3 me = position;

	if (numShadowRays > 1)
		me = randSphere(me, 0.3f);

	Vec3 dir = me - at;
	dir.normalize();

	return dir;
}
