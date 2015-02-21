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

struct Ray {
    vec3 origin;        //!< Ray origin point
    vec3 direction;     //!< Ray direction vector
    vec3 inv_direction; //!< 1 / ray direction components

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
          direction(direction),
          inv_direction(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z) // TODO SSE
    {
    }

    /**
     * @brief Get the point a distance 't' along the direction vector from the origin
     */
    inline vec3 at(float t) const {
        return origin + direction * t;
    }
};

#endif
