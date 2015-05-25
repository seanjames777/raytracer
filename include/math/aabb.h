/**
 * @file math/aabb.h
 *
 * @brief Axis-aligned bounding box
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_AABB_H
#define __MATH_AABB_H

#include <math/vector.h>
#include <math/ray.h>

struct AABB {
private:

    /**
     * @brief Swap the contents of two float pointers
     */
    inline static void swap(float *a, float *b) {
        float temp = *a;
        *a = *b;
        *b = temp;
    }

public:

    vec3 min; //!< Minimum point
    vec3 max; //!< Maximum point

    /**
     * @brief Constructor
     */
    AABB() {
    }

    /**
     * @brief Constructor
     */
    AABB(const vec3 & min, const vec3 & max)
        : min(min),
          max(max)
    {
    }

    /**
     * @brief Extend the bounds of the box to contain a new point
     */
    inline void join(const vec3 & pt) {
        min.x = fminf(min.x, pt.x);
        min.y = fminf(min.y, pt.y);
        min.z = fminf(min.z, pt.z);

        max.x = fmaxf(max.x, pt.x);
        max.y = fmaxf(max.y, pt.y);
        max.z = fmaxf(max.z, pt.z);
    }

    /**
     * @brief Extend the bounds of the box to contain another box
     */
    inline void join(const AABB & box) {
        join(box.min);
        join(box.max);
    }

    /**
     * @brief Check whether a ray intersects the box
     *
     * @param[in]  r        Ray to test against
     * @param[out] tmin_out Distance along ray to closest intersection
     * @param[out] tmax_out Distance along ray to furthest intersection
     *
     * @return Whether the ray intersected the box
     */
    // TODO test inline
    inline bool intersects(const Ray & r, float *tmin_out, float *tmax_out) const {
        // http://people.csail.mit.edu/amy/papers/box-jgt.pdf

        vec3 inv_direction = r.invDirection();

        float tmin, tmax;

        float txmin = (min.x - r.origin.x) * inv_direction.x;
        float txmax = (max.x - r.origin.x) * inv_direction.x;
        if (txmin > txmax) swap(&txmin, &txmax);

        float tymin = (min.y - r.origin.y) * inv_direction.y;
        float tymax = (max.y - r.origin.y) * inv_direction.y;
        if (tymin > tymax) swap(&tymin, &tymax);

        if (txmin > tymax || tymin > txmax) return false;

        tmin = fmaxf(txmin, tymin);
        tmax = fminf(txmax, tymax);

        float tzmin = (min.z - r.origin.z) * inv_direction.z;
        float tzmax = (max.z - r.origin.z) * inv_direction.z;
        if (tzmin > tzmax) swap(&tzmin, &tzmax);

        if (tmin > tzmax || tzmin > tmax)
            return false;

        tmin = fmaxf(tmin, tzmin);
        tmax = fminf(tmax, tzmax);

        *tmin_out = tmin;
        *tmax_out = tmax;

        return true;
    }

    /**
     * @brief Check whether a point is within the bounds of the box
     */
    inline bool contains(const vec3 & vec) const {
        return (vec.x >= min.x && vec.x <= max.x &&
                vec.y >= min.y && vec.y <= max.y &&
                vec.z >= min.z && vec.z <= max.z);
    }

    /**
     * @brief Check whether this box overlaps another on a specific axis
     *
     * @param[in] other Box to test against
     * @param[in] dir   Axis to test: 0, 1, 2 -> X, Y, Z
     */
    inline bool overlaps(const AABB & other, int dir) const {
        return !(
            max.v[dir] < other.min.v[dir] ||
            min.v[dir] > other.max.v[dir]);
    }

    /**
     * @brief Get the center point of the box
     */
    inline vec3 center() const {
        return min + (max - min) * .5f;
    }

    /**
     * @brief Calculate the surface area of this bounding box
     */
    inline float surfaceArea() const {
        vec3 ext = max - min;
        return 2 * (ext.x * ext.y + ext.x * ext.z + ext.y * ext.z);
    }

    /**
     * @brief Split the box along a specific axis
     *
     * @param[in]  dist Split distance along axis from minimum point
     * @param[in]  axis Axis to split along: 0, 1, 2 -> X, Y, Z
     * @param[out] Box to the "left" of the split plane
     * @param[out] Box to the "right" of the split plane
     */
    AABB split(float dist, int axis, AABB & left, AABB & right) const {
        switch(axis) {
        case 0:
            left = AABB(min, vec3(dist, max.y, max.z));
            right = AABB(vec3(dist, min.y, min.z), max);
            break;
        case 1:
            left = AABB(min, vec3(max.x, dist, max.z));
            right = AABB(vec3(min.x, dist, min.z), max);
            break;
        case 2:
            left = AABB(min, vec3(max.x, max.y, dist));
            right = AABB(vec3(min.x, min.y, dist), max);
            break;
        }

        return AABB(); // Shouldn't happen
    }

};

#endif
