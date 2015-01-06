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
    vec3 direction;

    /** @brief Color */
    vec3 color;

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
    DirectionalLight(vec3 direction, vec3 color, bool shadow);

    /**
     * @brief Get the direction of the light
     */
    vec3 getDirection(const vec3 & pos);

    /**
     * @brief Get the color of the light
     */
    vec3 getColor(const vec3 & pos);

    /**
     * @brief Get whether this light casts shadows
     */
    bool castsShadows();

    /**
     * Get the direction of a shadow-testing ray from a given position
     */
    vec3 getShadowDir(const vec3 & at);
};

#endif
