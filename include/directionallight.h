/**
 * @file directionallight.h
 *
 * @brief Directional light source
 *
 * @author Sean James
 */

#ifndef _DIRECTIONALLIGHT_H
#define _DIRECTIONALLIGHT_H

#include <light.h>

/**
 * @brief A directional light source
 */
class DirectionalLight : public Light {
private:

	/** @brief Light direction */
	Vec3 direction;

	/** @brief Color */
	Vec3 color;

	/** @brief Whether this light casts shadows */
	bool shadow;

public:

	/**
	 * @brief Constructor
	 *
	 * @param direction Light direction (normalized)
	 * @param color     Color
	 * @param shadow    Whether this light casts shadows
	 */
	DirectionalLight(Vec3 direction, Vec3 color, bool shadow);

	/**
	 * @brief Get the direction of the light
	 */
	Vec3 getDirection(Vec3 pos);

	/**
	 * @brief Get the color of the light
	 */
	Vec3 getColor(Vec3 pos);

	/**
	 * @brief Get whether this light casts shadows
	 */
	bool castsShadows();

	/**
	 * Get the direction of a shadow-testing ray from a given position
	 */
	Vec3 getShadowDir(Vec3 at);
};

#endif
