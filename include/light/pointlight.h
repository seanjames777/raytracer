/**
 * @file pointlight.h
 *
 * @brief Point light
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef _POINTLIGHT_H
#define _POINTLIGHT_H

#include <light/light.h>

/**
 * @brief A point light with an origin, radius, and falloff curve
 */
class RT_EXPORT PointLight : public Light {
private:

    /** @brief Position */
    float3  position;

    /** @brief Color */
    float3 color;

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
    PointLight(float3 position, float3 color, float radius, float range, float power, bool shadow);

    /**
     * @brief Get the direction of the point light at a given point
     */
    float3 getDirection(const float3 & pos);

    /**
     * @brief Get the color of the point light at a given point
     */
    float3 getColor(const float3 & pos);

    /**
     * @brief Whether this point light casts shadows
     */
    bool castsShadows();

    /**
     * @brief Get the direction of a set of shadow testing rays. Samples are added to a vector to
     * allow for stratefied sampling and to allow lights to return variable numbers of samples.
     * Note that the number of samples returned may not be the same as nSamples. For example, a
     * point light with no radius does not need more than one sample, while stratification requires
     * perfect square numbers of samples.
     *
     * @param at       Location to sample from
     * @param samples  Vector of samples to add to
     * @param nSamples Number of samples to compute
     */
    void getShadowDir(const float3 & at, float3 *samples, int nSamples);

};

#endif
