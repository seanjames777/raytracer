/**
 * @file light.h
 *
 * @brief Base class for lights
 *
 * @author Sean James
 */

#ifndef _LIGHT_H
#define _LIGHT_H

#include <defs.h>
#include <rtmath.h>

/**
 * @brief Base class for lights
 */
class Light {
public:

	/**
	 * @brief Get the direction of the light at a certain point
	 */
	virtual Vec3 getDirection(Vec3 pos)  = 0;

	/**
	 * @brief Get the color of the light at a certain point
	 */
	virtual Vec3 getColor(Vec3 pos) = 0;

	/**
	 * @brief Get whether this light casts shadows
	 */
	virtual bool castsShadows() = 0;

	/**
	 * @brief Get the direction of a shadow-testing ray starting at
	 * the given point and pointing towards the light
	 */
	virtual Vec3 getShadowDir(Vec3 at) = 0;
};

#endif
