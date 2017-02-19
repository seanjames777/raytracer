#ifndef __MATH_PLANE_H
#define __MATH_PLANE_H

#include <math/ray.h>

struct Plane {

    float3 normal;
    float distance;

    Plane()
        : distance(0)
    {
    }

    Plane(float3 normal, float distance)
        : normal(normal),
          distance(distance)
    {
    }

    inline bool intersects(const Ray & ray, float & t) const {
        // O = Ray origin
        // D = Ray direction
        // t = Intersection distance
        // P = Intersection point
        // N = plane normal
        // d = Plane distance from origin along normal
        //
        // P = O + t * D
        // Let P' = d * N
        //
        // We want to solve for t
        //
        // P' and P lie in plane, so (P - P') . N = 0
        // (O + tD - dN) . N = 0
        // O . N + t (D . N) - d (N . N) = 0
        //
        // N is a normal, so N . N = 1
        //
        // O . N + t (D . N) - d = 0
        //
        // t = (d - O . N) / (D . N)
        //
        // Note D . N must not be 0. If t < 0, intersection is behind camera. Works whether normal faces
        // away from ray origin or not.

        float denom = dot(ray.direction, normal);

        if (denom == 0.0f)
            return false;

        t = (distance - dot(ray.origin, normal)) / denom;

        if (t < 0.0f)
            return false;

        return true;
    }

};

#endif
