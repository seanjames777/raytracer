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
	// TODO decide how to handle aligns and the W component
	vec3 origin;        //!< Ray origin point
	vec3 direction;     //!< Ray direction vector
	vec3 inv_direction; //!< 1 / ray direction components

	__m128 ox, oy, oz;
	__m128 dx, dy, dz;

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
		ox = _mm_set1_ps(origin.x);
		oy = _mm_set1_ps(origin.y);
		oz = _mm_set1_ps(origin.z);

		dx = _mm_set1_ps(direction.x);
		dy = _mm_set1_ps(direction.y);
		dz = _mm_set1_ps(direction.z);
    }

    /**
     * @brief Get the point a distance 't' along the direction vector from the origin
     */
    inline vec3 at(float t) const {
        return origin + direction * t;
    }
};

#endif
