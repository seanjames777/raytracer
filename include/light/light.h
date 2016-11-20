/**
 * @file llight/ight.h
 *
 * @brief Base class for lights
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __LIGHT_H
#define __LIGHT_H

#include <rt_defs.h>

/**
 * @brief Base class for lights
 */
class RT_EXPORT Light {
public:

    /**
     * @brief Get the direction of the light at a certain point
     */
    virtual float3 getDirection(const float3 & pos)  = 0;

    /**
     * @brief Get the color of the light at a certain point
     */
    virtual float3 getColor(const float3 & pos) = 0;

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
    virtual void getShadowDir(const float3 & at, float3 *samples, int nSamples) = 0;
};

#endif
