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
	vec3  origin;        //!< 12 Ray origin point
	vec3  direction;     //!< 12 Ray direction vector
    vec3  weight;        //!< 12 Ray contribution weight/color
    vec2  deriv;         //!<  8 Screen space derivative for texture sampling
    short px;            //!<  2 Ray contribution pixel X
    short py;            //!<  2 Ray contribution pixel Y
    char  depth;         //!<  1 Ray recursion depth. TODO: 49 bytes is weird. Maybe russian roulette or something else.
                         //   49

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
          weight(1.0f),
          px(0),
          py(0)
    {
    }

    /**
     * @brief Constructor
     *
     * @param[in] origin    Ray origin point
     * @param[in] direction Ray direction vector
     */
    Ray(const vec3 & origin, const vec3 & direction, const vec3 & weight, short px, short py, char depth)
        : origin(origin),
          direction(direction),
          weight(weight),
          px(px),
          py(py),
          depth(depth)
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
