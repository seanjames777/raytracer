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

    float3 position;
    float radius;
    float3 radiance;
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
    PointLight(const float3 & position, float radius, const float3 & radiance, bool shadow);

    float3 getPosition() const {
        return position;
    }

    float getRadius() const {
        return radius;
    }

    float3 getRadiance() const {
        return radiance;
    }

	virtual void sample(const float3 & uv, const float3 & p, float3 & wo, float & r, float3 & Lo) const;

	/**
	* @brief Get whether this light casts shadows
	*/
	bool castsShadows() const;

};

#endif
