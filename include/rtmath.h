/**
 * @file rtmath.h
 *
 * @brief Raytracer 3D math library
 *
 * @author Sean James
 */

#ifndef _RTMATH_H
#define _RTMATH_H

#include <math/vector.h>
#include <math/matrix.h>
#include <vector>
#include <stdlib.h>

#define MIN2(x, y) ((x) < (y) ? (x) : (y))
#define MAX2(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(x, min, max) (MAX2(MIN2(x, max), min))
#define SATURATE(x) (CLAMP(x, 0.0f, 1.0f))
#define SIGN(x) ((x) > 0 ? 1 : -1)
#define SIGNF(x) ((x) > 0.0f ? 1.0f : -1.0f)
#define INFINITY32F ((float)0x7F800000)

// namespace math { // TODO

/**
 * A ray with an origin and a direction
 */
struct Ray {

    vec3 origin;        // Origin of the ray
    vec3 direction;     // Direction of the ray
    vec3 inv_direction; // 1 / ray direction
    vec3 d_origin;
    vec3 d_direction;

    /**
     * Empty constructor
     */
    Ray() {
    }

    /*
     * Constructor accepts an origin and a direction
     */
    Ray(vec3 origin, vec3 direction)
        : origin(origin),
          direction(direction),
          inv_direction(vec3(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z))
    {
    }

    /*
     * Get the point a distance t from the origin in the direction
     * 'direction'
     */
    inline vec3 at(float t) {
        vec3 b = direction * t;
        return origin + b;
    }
};

/*
 * An axis-aligned bounding box
 */
struct AABB {
private:

    /*
     * Swap the 32 bit floating point numbers at two locations
     */
    inline static void swap(float *a, float *b) {
        float temp = *a;
        *a = *b;
        *b = temp;
    }

public:

    vec3 _min;  // The minimum point of the AABB
    vec3 _max;  // The maximum point of the AABB

    /*
     * Empty constructor does not generate a valid bounding box,
     * only useful for array initialization
     */
    AABB() {
    }

    /*
     * Constructor accepts minimum and maximum points
     */
    AABB(const vec3 & Min, const vec3 & Max)
        : _min(Min), _max(Max)
    {
        join(Min);
        join(Max);
    }

    /*
     * Add a point to the bounding box, adjusting the min and
     * max points as necessary
     */
    void join(const vec3 & pt) {
		_min.x = MIN2(_min.x, pt.x);
		_min.y = MIN2(_min.y, pt.y);
		_min.z = MIN2(_min.z, pt.z);

		_max.x = MAX2(_max.x, pt.x);
		_max.y = MAX2(_max.y, pt.y);
        _max.z = MAX2(_max.z, pt.z);
    }

    /*
     * Add the range specified by another bounding box, adjusting
     * the min and max points as necessary
     */
    void join(const AABB & box) {
		join(box._min);
		join(box._max);
    }

    /*
     * Check whether the given ray intersects this AABB, setting
     * the nearest and farthest intersection points in tmin_out
     * and tmax_out
     */
    bool intersects(const Ray & r, float *tmin_out, float *tmax_out) const {
        // http://people.csail.mit.edu/amy/papers/box-jgt.pdf

        float tmin, tmax;

		float txmin = (_min.x - r.origin.x) * r.inv_direction.x;
		float txmax = (_max.x - r.origin.x) * r.inv_direction.x;
        if (txmin > txmax) swap(&txmin, &txmax);

		float tymin = (_min.y - r.origin.y) * r.inv_direction.y;
		float tymax = (_max.y - r.origin.y) * r.inv_direction.y;
        if (tymin > tymax) swap(&tymin, &tymax);

        if (txmin > tymax || tymin > txmax) return false;

        tmin = MAX2(txmin, tymin);
        tmax = MIN2(txmax, tymax);

		float tzmin = (_min.z - r.origin.z) * r.inv_direction.z;
		float tzmax = (_max.z - r.origin.z) * r.inv_direction.z;
        if (tzmin > tzmax) swap(&tzmin, &tzmax);

        if (tmin > tzmax || tzmin > tmax)
            return false;

        tmin = MAX2(tmin, tzmin);
        tmax = MIN2(tmax, tzmax);

        *tmin_out = tmin;
        *tmax_out = tmax;

        return true;
    }

    /*
     * Whether this AABB contains a given point
     */
    bool contains(const vec3 & vec) const {
		return (vec.x >= _min.x && vec.x <= _max.x &&
			    vec.y >= _min.y && vec.y <= _max.y &&
			    vec.z >= _min.z && vec.z <= _max.z);
    }

    /*
     * Whether this bounding box overlaps another at all
     */
    bool overlaps(const AABB & other, int dir) const {
        // TODO: branch?
        return !(
			_max.v[dir] < other._min.v[dir] ||
			_min.v[dir] > other._max.v[dir]);
    }

    /*
     * Get the center point of this bounding box
     */
    vec3 center() const {
		return _min + (_max - _min) * .5f;
    }

    /*
     * Calculate the surface area of this bounding box
     */
    float surfaceArea() const {
		vec3 ext = _max - _min;
        return 2 * (ext.x * ext.y + ext.x * ext.z + ext.y * ext.z);
    }

    /*
     * Split the bounding box
     */
    AABB split(float dist, int axis, AABB & left, AABB & right) const {
        switch(axis) {
        case 0:
			left = AABB(_min, vec3(dist, _max.y, _max.z));
			right = AABB(vec3(dist, _min.y, _min.z), _max);
            break;
        case 1:
			left = AABB(_min, vec3(_max.x, dist, _max.z));
			right = AABB(vec3(_min.x, dist, _min.z), _max);
            break;
        case 2:
			left = AABB(_min, vec3(_max.x, _max.y, dist));
			right = AABB(vec3(_min.x, _min.y, dist), _max);
            break;
        }

        return AABB(); // Shouldn't happen
    }
};

// TODO
#define randf(min, max) (min + ((float)rand() / (float)RAND_MAX) * (max - min))

void randSphere(vec3 *samples, int sqrtSamples);
vec2 randCircle(float rad);
void randHemisphereCos(vec3 norm, vec3 *samples, int sqrtSamples, float radiusScale);

// };

#endif
