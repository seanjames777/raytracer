/*
 * Sean James
 *
 * directionallight.cpp
 *
 * Implementation of a directional light source
 *
 */

#include "directionallight.h"

/*
 * Constructor accepts light parameters
 */
DirectionalLight::DirectionalLight(const Vec3 & Direction, const Color & Color, bool Shadow)
	: direction(Direction), 
	  color(Color), 
	  shadow(Shadow)
{
	direction.normalize();
}

/*
 * Get the direction of the light
 */
Vec3 DirectionalLight::getDirection(const Vec3 & pos) {
	return direction;
}

/*
 * Get the color of the light
 */
Color DirectionalLight::getColor(const Vec3 & pos) {
	return color;
}

/*
 * Get whether this light casts shadows
 */
bool DirectionalLight::castsShadows() {
	return shadow;
}

/*
 * Get the direction of a shadow-testing ray from a
 * given position, randomly jittered if soft shadows
 * are enabled
 */
Vec3 DirectionalLight::getShadowDir(const Vec3 & at, int numShadowRays) {
	Vec3 d = -direction;
	
	if (numShadowRays > 1)
		d = d + randSphere(Vec3(), .02f);

	d.normalize();

	return d;
}
