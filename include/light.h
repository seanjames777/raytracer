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
 * @brief Photon
 */
struct Photon {
	/** @brief Photon position */
	Vec3 position;

	/** @brief Red, green, blue power */
	Vec3 power;

	/** @brief Incident direction */
	Vec3 direction;

	/**
	 * @brief Constructor
	 *
	 * @param position  Photon position
	 * @param power     Red, green, blue power
	 * @param direction Incident direction
	 */
	Photon(Vec3 position, Vec3 power, Vec3 direction);
};

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
