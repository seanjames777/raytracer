/**
 * @file pointlight.h
 *
 * @brief Point light
 *
 * @author Sean James
 */

#ifndef _POINTLIGHT_H
#define _POINTLIGHT_H

#include <light.h>

/**
 * @brief A point light with an origin, radius, and falloff curve
 */
class PointLight : public Light {
private:

	/** @brief Position */
	Vec3  position;

	/** @brief Color */
	Vec3 color;

	/** @brief Radius */
	float radius;

	/** @brief Squared range */
	float range2;

	/** @brief Exponent of falloff curve */
	float power;

	/** @brief Whether this light casts shadows */
	bool shadow;

public:

	/**
	 * @brief Constructor
	 *
	 * @param position Position of the light
	 * @param color    Color of the light
	 * @param radius   Radius of the light. 0 for a point light.
	 * @param range    Maximum lit distance for falloff curve
	 * @param power    Exponent of the falloff curve
	 * @param shadow   Whether this light casts shadows
	 */
	PointLight(Vec3 position, Vec3 color, float radius, float range, float power, bool shadow);

	/**
	 * @brief Get the direction of the point light at a given point
	 */
	Vec3 getDirection(Vec3 pos);

	/**
	 * @brief Get the color of the point light at a given point
	 */
	Vec3 getColor(Vec3 pos);

	/**
	 * @brief Whether this point light casts shadows
	 */
	bool castsShadows();

	/**
	 * @brief Get the direction of a shadow-testing ray at a given point
	 */
	Vec3 getShadowDir(Vec3 at);
};

#endif
