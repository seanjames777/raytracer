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
#include <math/vector.h>

/**
 * @brief Base class for lights
 */
class RT_EXPORT Light {
public:

	virtual void sample(const float3 & uv, const float3 & p, float3 & wo, float & r, float3 & Lo) const = 0;

    /**
     * @brief Get whether this light casts shadows
     */
    virtual bool castsShadows() const = 0;

};

#endif
