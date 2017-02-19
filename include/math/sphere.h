#ifndef __MATH_SPHERE_H
#define __MATH_SPHERE_H

#include <math/ray.h>

struct Sphere {

    float3 origin;
    float radius;

    Sphere()
        : radius(0)
    {
    }

    Sphere(float3 origin, float radius)
        : origin(origin),
          radius(radius)
    {
    }

    // Solves quadratic equation in a way which avoids catastrophic cancellation
    // between -b and root terms. Also handles case of 0 or 1 roots.
    inline static bool quadratic(float a, float b, float c, float & t1, float & t2) {
        float disc = b * b - 4 * a * c;

        if (disc < 0.0f)
            return false;

        if (disc == 0.0f) {
            t1 = t2 = -0.5f * b / a;
            return true;
        }

        float s = (b < 0.0f ? -1.0f : 0.0f);
        float q = -0.5f * (b + s * sqrtf(disc));

        t1 = q / a;
        t2 = c / q;

        return true;
    }

    inline bool intersects(const Ray & ray, float & t) const {
        // (a + b + c)(a + b + c)
        // O = Ray origin
        // D = Ray direction
        // t = Intersection distance
        // P = Intersection point
        // C = Sphere origin
        // r = Sphere radius
        //
        // P = O + tD
        // | P - C | = r
        //
        // We want to solve for t. Multiplying out terms and simplifying:
        // 
        // t^2 + 2tD.(O - C) + |O - C|^2 = r^2
        //
        // Let a = 1
        //     b = 2D.(O - C)
        //     c = |O-C|^2 - r^2
        //
        // t^2 + b t + c = 0, which can be solved by the quadratic equation.
        // Note that b^2 - 4ac must be > 0.

        float3 v = ray.origin - origin;

        float b = 2.0f * dot(ray.direction, v);
        float c = dot(v, v) - radius * radius;

        float t1, t2;

        if (!quadratic(1.0f, b, c, t1, t2))
            return false;

        // Put roots in order
        if (t1 > t2)
            swap(t1, t2);

        // If nearest root is behind camera, use further root
        if (t1 < 0.0f) {
            t1 = t2;

            // If further root is behind camera, no intersection
            if (t1 < 0.0f)
                return false;
        }

        t = t1;
        return true;
    }

};

#endif
