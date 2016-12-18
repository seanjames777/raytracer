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
    inline static void swap(THREAD T & a, THREAD T & b) {
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
    explicit AABB() {
    }

    /**
     * @brief Constructor
     */
    explicit AABB(const float3 & pt)
        : min(pt),
          max(pt)
    {
    }

    /**
     * @brief Constructor
     */
    explicit AABB(const float3 & min, const float3 & max)
        : min(min),
          max(max)
    {
    }

    /**
     * @brief Extend the bounds of the box to contain a new point
     */
    inline void join(const float3 & pt) {
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
    inline bool intersects(
		THREAD const float3 & origin,
		THREAD const float3 & inv_direction,
		THREAD float & tmin_out,
		THREAD float & tmax_out) const
    {
        // http://people.csail.mit.edu/amy/papers/box-jgt.pdf

        float tmin, tmax;
		bool result = true;

        float _txmin = (min[0] - origin[0]) * inv_direction[0];
        float _txmax = (max[0] - origin[0]) * inv_direction[0];
		float txmin = ::min(_txmin, _txmax);
		float txmax = ::max(_txmin, _txmax);

        float _tymin = (min[1] - origin[1]) * inv_direction[1];
        float _tymax = (max[1] - origin[1]) * inv_direction[1];
		float tymin = ::min(_tymin, _tymax);
		float tymax = ::max(_tymin, _tymax);

        result = result && !(txmin > tymax || tymin > txmax);

        tmin = ::max(txmin, tymin);
        tmax = ::min(txmax, tymax);

        float _tzmin = (min[2] - origin[2]) * inv_direction[2];
        float _tzmax = (max[2] - origin[2]) * inv_direction[2];
		float tzmin = ::min(_tzmin, _tzmax);
		float tzmax = ::max(_tzmin, _tzmax);

		result = result && !(tmin > tzmax || tzmin > tmax);

        tmin = ::max(tmin, tzmin);
        tmax = ::min(tmax, tzmax);

        tmin_out = tmin;
        tmax_out = tmax;

        return result;
    }

	template<unsigned int N>
	inline vector<bmask, N> intersectsPacket(
		THREAD const vector<float, N> (&origin)[3],
		THREAD const vector<float, N> (&inv_direction)[3],
		THREAD vector<float, N> & tmin_out,
		THREAD vector<float, N> & tmax_out) const
	{
		// http://people.csail.mit.edu/amy/papers/box-jgt.pdf

		vector<float, N> tmin, tmax;
		vector<bmask, N> result = vector<bmask, N>(0xFFFFFFFF);

		// Broadcast to all channels. TODO: could do this at construction.
		vector<float, N> minv[3] = { min.x, min.y, min.z };
		vector<float, N> maxv[3] = { max.x, max.y, max.z };

		vector<float, N> _txmin = (minv[0] - origin[0]) * inv_direction[0];
		vector<float, N> _txmax = (maxv[0] - origin[0]) * inv_direction[0];
		vector<float, N> txmin = ::min(_txmin, _txmax);
		vector<float, N> txmax = ::max(_txmin, _txmax);

		vector<float, N> _tymin = (minv[1] - origin[1]) * inv_direction[1];
		vector<float, N> _tymax = (maxv[1] - origin[1]) * inv_direction[1];
		vector<float, N> tymin = ::min(_tymin, _tymax);
		vector<float, N> tymax = ::max(_tymin, _tymax);

		result = result & ~((txmin > tymax) | (tymin > txmax));

		tmin = ::max(txmin, tymin);
		tmax = ::min(txmax, tymax);

		vector<float, N> _tzmin = (minv[2] - origin[2]) * inv_direction[2];
		vector<float, N> _tzmax = (maxv[2] - origin[2]) * inv_direction[2];
		vector<float, N> tzmin = ::min(_tzmin, _tzmax);
		vector<float, N> tzmax = ::max(_tzmin, _tzmax);

		result = result & ~((tmin > tzmax) | (tzmin > tmax));

		tmin = ::max(tmin, tzmin);
		tmax = ::min(tmax, tzmax);

		tmin_out = tmin;
		tmax_out = tmax;

		return result;
	}

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

    /**
     * @brief Split the box along a specific axis
     *
     * @param[in]  dist  Split distance along axis from minimum point
     * @param[in]  axis  Axis to split along: 0, 1, 2 -> X, Y, Z
     * @param[out] left  Box to the "left" of the split plane
     * @param[out] right Box to the "right" of the split plane
     */
    AABB split(float dist, int axis, THREAD AABB & left, THREAD AABB & right) const {
        switch(axis) {
        case 0:
            left = AABB(min, float3(dist, max.y, max.z));
            right = AABB(float3(dist, min.y, min.z), max);
            break;
        case 1:
            left = AABB(min, float3(max.x, dist, max.z));
            right = AABB(float3(min.x, dist, min.z), max);
            break;
        case 2:
            left = AABB(min, float3(max.x, max.y, dist));
            right = AABB(float3(min.x, min.y, dist), max);
            break;
        }

        return AABB(); // Shouldn't happen
    }

};

inline std::ostream & operator<<(std::ostream & os, const AABB & aabb) {
    return os << "{ min: " << aabb.min << ", max: " << aabb.max << " }";
}

#endif
