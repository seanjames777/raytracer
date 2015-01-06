/**
 * @file light.h
 *
 * @brief Base class for lights
 *
 * @author Sean James
 */

#ifndef _LIGHT_H
#define _LIGHT_H

#include <rtmath.h>
#include <raytracersettings.h>

/**
 * @brief Photon
 */
struct Photon {
    /** @brief Photon position */
    vec3 position;

    /** @brief Red, green, blue power */
    vec3 power;

    /** @brief Incident direction */
    vec3 direction;

    /**
     * @brief Constructor
     *
     * @param position  Photon position
     * @param power     Red, green, blue power
     * @param direction Incident direction
     */
    Photon(vec3 position, vec3 power, vec3 direction);
};

/**
 * @brief Base class for lights
 */
class Light {
public:

    /**
     * @brief Get the direction of the light at a certain point
     */
    virtual vec3 getDirection(const vec3 & pos)  = 0;

    /**
     * @brief Get the color of the light at a certain point
     */
    virtual vec3 getColor(const vec3 & pos) = 0;

    /**
     * @brief Get whether this light casts shadows
     */
    virtual bool castsShadows() = 0;

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
    virtual void getShadowDir(const vec3 & at, vec3 *samples, int nSamples) = 0;
};

#endif
