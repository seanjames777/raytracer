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
#include <xmmintrin.h>
#include <smmintrin.h>

struct Ray {
	vec3 origin;        //!< Ray origin point
	vec3 direction;     //!< Ray direction vector

    // TODO: Derivatives for sampling

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
    Ray(const vec3 & origin, const vec3 & direction)
        : origin(origin),
          direction(direction)
    {
    }

    /**
     * @brief Get the point a distance 't' along the direction vector from the origin
     */
    inline vec3 at(float t) const {
        return origin + direction * t;
    }

    /**
     * @brief Get inverse ray direction
     */
    inline vec3 invDirection() const {
        return vec3(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);
    }
};

#endif
