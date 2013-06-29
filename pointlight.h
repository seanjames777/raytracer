/*
 * Sean James
 *
 * pointlight.h
 *
 * Point light with an origin, radius, and falloff curve
 *
 */

#ifndef _POINTLIGHT_H
#define _POINTLIGHT_H

#include "light.h"

/*
 * A point light with an origin, radius, and falloff curve
 */
class PointLight : public Light {
public:
	Vec3  position;        // Position of the light
	Color color;           // Color of the light
	float radius2;         // Squared radius of the light
	float power;           // Exponent of the falloff curve
	bool  shadow;          // Whether this light casts shadows

	/*
	 * Constructor accepts light parameters
	 */
	PointLight(const Vec3 & Position, const Color & Color, float Radius, float Power, bool Shadow);

	/*
	 * Get the direction of the point light at a given point
	 */
	Vec3 getDirection(const Vec3 & pos);

	/*
	 * Get the color of the point light at a given point
	 */
	Color getColor(const Vec3 & pos);

	/*
	 * Whether this point light casts shadows
	 */
	bool castsShadows();

	/*
	 * Get the direction of a shadow-testing ray at a given
	 * point, randomly jittered if soft-shadows are enabled
	 */
	Vec3 getShadowDir(const Vec3 & at, int numShadowRays);
};

#endif
