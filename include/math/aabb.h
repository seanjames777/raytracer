/**
 * @file math/aabb.h
 *
 * @brief Axis-aligned bounding box
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_AABB_H
#define __MATH_AABB_H

#include <math/ray.h>

struct AABB {
private:

#if GPU
    /**
     * @brief Swap the contents of two float pointers
     */
    template<typename T>
    inline static void swap(thread T & a, thread T & b) {
        T temp(a);
        a = b;
        b = temp;
    }
#endif

public:

    float3 min; //!< Minimum point
    float3 max; //!< Maximum point

    /**
     * @brief Constructor
     */
    AABB() {
    }

    /**
     * @brief Constructor
     */
    AABB(float3 pt)
        : min(pt),
          max(pt)
    {
    }

    /**
     * @brief Constructor
     */
    AABB(float3 min, float3 max)
        : min(min),
          max(max)
    {
    }

    /**
     * @brief Extend the bounds of the box to contain a new point
     */
    inline void join(float3 pt) {
        min.x = fmin(min.x, pt.x);
        min.y = fmin(min.y, pt.y);
        min.z = fmin(min.z, pt.z);

        max.x = fmax(max.x, pt.x);
        max.y = fmax(max.y, pt.y);
        max.z = fmax(max.z, pt.z);
    }

    /**
     * @brief Extend the bounds of the box to contain another box
     */
    inline void join(AABB box) {
        join(box.min);
        join(box.max);
    }

#if GPU
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
    inline bool intersects(Ray r, thread float & tmin_out, thread float & tmax_out) const
    {
        // http://people.csail.mit.edu/amy/papers/box-jgt.pdf

        float3 inv_direction = r.invDirection();

        float tmin, tmax;

        float txmin = (min.x - r.origin.x) * inv_direction.x;
        float txmax = (max.x - r.origin.x) * inv_direction.x;
        if (txmin > txmax) swap(txmin, txmax);

        float tymin = (min.y - r.origin.y) * inv_direction.y;
        float tymax = (max.y - r.origin.y) * inv_direction.y;
        if (tymin > tymax) swap(tymin, tymax);

        if (txmin > tymax || tymin > txmax) return false;

        tmin = fmax(txmin, tymin);
        tmax = fmin(txmax, tymax);

        float tzmin = (min.z - r.origin.z) * inv_direction.z;
        float tzmax = (max.z - r.origin.z) * inv_direction.z;
        if (tzmin > tzmax) swap(tzmin, tzmax);

        if (tmin > tzmax || tzmin > tmax)
            return false;

        tmin = fmax(tmin, tzmin);
        tmax = fmin(tmax, tzmax);

        tmin_out = tmin;
        tmax_out = tmax;

        return true;
    }
#endif

    /**
     * @brief Check whether a point is within the bounds of the box
     */
    inline bool contains(float3 vec) const {
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
    inline bool overlaps(AABB other, int dir) const {
        return !(
            max[dir] < other.min[dir] ||
            min[dir] > other.max[dir]);
    }

    /**
     * @brief Get the center point of the box
     */
    inline float3 center() const {
        return min + (max - min) * .5f;
    }

    /**
     * @brief Calculate the surface area of this bounding box
     */
    inline float surfaceArea() const {
        float3 ext = max - min;
        return 2 * (ext.x * ext.y + ext.x * ext.z + ext.y * ext.z);
    }

#if !GPU
    /**
     * @brief Split the box along a specific axis
     *
     * @param[in]  dist  Split distance along axis from minimum point
     * @param[in]  axis  Axis to split along: 0, 1, 2 -> X, Y, Z
     * @param[out] left  Box to the "left" of the split plane
     * @param[out] right Box to the "right" of the split plane
     */
    AABB split(float dist, int axis, AABB & left, AABB & right) const {
        switch(axis) {
        case 0:
            left = AABB(min, (float3){ dist, max.y, max.z });
            right = AABB((float3){ dist, min.y, min.z }, max);
            break;
        case 1:
            left = AABB(min, (float3){ max.x, dist, max.z });
            right = AABB((float3){ min.x, dist, min.z }, max);
            break;
        case 2:
            left = AABB(min, (float3){ max.x, max.y, dist });
            right = AABB((float3){ min.x, min.y, dist }, max);
            break;
        }

        return AABB(); // Shouldn't happen
    }
#endif

};

#endif
