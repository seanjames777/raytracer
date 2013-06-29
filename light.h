/*
 * Sean James
 *
 * light.h
 *
 * Base class/interface for lights
 *
 */

#ifndef _LIGHT_H
#define _LIGHT_H

#include "defs.h"
#include "rtmath.h"

/*
 * Base class for lights
 */
class Light
{
public:

	/*
	 * Get the direction of the light at a certain point
	 */
	virtual Vec3 getDirection(const Vec3 & pos)  = 0;

	/*
	 * Get the color of the light at a certain point
	 */
	virtual Color getColor(const Vec3 & pos) = 0;

	/*
	 * Get whether this light casts shadows
	 */
	virtual bool castsShadows() = 0;

	/*
	 * Get the direction of a shadow-testing ray starting at
	 * the given point and pointing towards the light
	 */
	virtual Vec3 getShadowDir(const Vec3 & at, int numShadowRays) = 0;
};

#endif
