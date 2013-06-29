/*
 * Sean James
 *
 * directionallight.h
 *
 * Interface for a directional light source
 *
 */

#ifndef _DIRECTIONALLIGHT_H
#define _DIRECTIONALLIGHT_H

#include "light.h"

/*
 * A directional light source
 */
class DirectionalLight : public Light {
public:
	Vec3 direction;  // Direction of the light
	Color color;     // Color of the light
	bool shadow;     // Whether the light casts shadows

	/*
	 * Constructor accepts light parameters
	 */
	DirectionalLight(const Vec3 & Direction, const Color & Color, bool Shadow);

	/*
	 * Get the direction of the light
	 */
	Vec3 getDirection(const Vec3 & pos);

	/*
	 * Get the color of the light
	 */
	Color getColor(const Vec3 & pos);

	/*
	 * Get whether this light casts shadows
	 */
	bool castsShadows();

	/*
	 * Get the direction of a shadow-testing ray from a
	 * given position, randomly jittered if soft shadows
	 * are enabled
	 */
	Vec3 getShadowDir(const Vec3 & at, int numShadowRays);
};

#endif
