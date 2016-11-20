/**
 * @file math/ray.h
 *
 * @brief Ray type
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_RAY_H
#define __MATH_RAY_H

#include <math/vector.h>

enum RayMode {
    Shade     = 0,
    Shadow    = 1
};

struct Ray {
    float3  origin;        //!< 12 Ray origin point
    float3  direction;     //!< 12 Ray direction vector

    /**
     * @brief Constructor
     */
    Ray() {
    }

    /**
     * @brief Constructor
     *
     * @param[in] origin    Ray origin point
     * @param[in] direction Ray direction vector
     */
    Ray(float3 origin, float3 direction)
        : origin(origin),
          direction(direction)
    {
    }

    /**
     * @brief Get the point a distance 't' along the direction vector from the origin
     */
    inline float3 at(float t) const {
        return origin + direction * t;
    }

    /**
     * @brief Get inverse ray direction
     */
    inline float3 invDirection() const {
        return float3(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);
    }
};

#endif
