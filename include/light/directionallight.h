/**
 * @file light/directionallight.h
 *
 * @brief Directional light source
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __DIRECTIONALLIGHT_H
#define __DIRECTIONALLIGHT_H

#include <light/light.h>

/**
 * @brief A directional light source
 */
class DirectionalLight : public Light {
private:

    /** @brief Light direction */
    float3 direction;

    /** @brief Color */
    float3 color;

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
    DirectionalLight(float3 direction, float3 color, bool shadow);

    /**
     * @brief Get the direction of the light
     */
    float3 getDirection(const float3 & pos);

    /**
     * @brief Get the color of the light
     */
    float3 getColor(const float3 & pos);

    /**
     * @brief Get whether this light casts shadows
     */
    bool castsShadows();

    /**
     * Get the direction of a shadow-testing ray from a given position
     */
    float3 getShadowDir(const float3 & at);
};

#endif
