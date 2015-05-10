/**
 * @file polygon.h
 *
 * @brief Polygon shape
 *
 * @author Sean James
 */

#ifndef _POLYGON_H
#define _POLYGON_H

#include <math/aabb.h>
#include <rt_defs.h>

//#define WALD_INTERSECTION
#define MOLLER_TRUMBORE_INTERSECTION

struct RT_EXPORT Vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec4 color;

    Vertex();

    Vertex(vec3 position, vec3 normal, vec2 uv, vec4 color);
};

struct RT_EXPORT Triangle {
    // TODO 0-3
    Vertex       v1;
    Vertex       v2;
    Vertex       v3;
    vec3         normal;
    AABB         bbox;
    unsigned int triangle_id;

    // TODO other constructors

    Triangle(Vertex v1, Vertex v2, Vertex v3);

    Vertex interpolate(float beta, float gamma);
};

struct Collision {
    float distance;
    float beta;
    float gamma;
    unsigned int triangle_id;
};

struct RT_EXPORT SetupTriangle {
#if defined(WALD_INTERSECTION)
    float n_u; // normal.u / normal.k
    float n_v; // normal.v / normal.h
    float n_d; // constant of plane equation
    int k;     // projection axis

    // line equation AC
    float b_nu;
    float b_nv;
    float b_d;

    // line equation AB
    float c_nu;
    float c_nv;
    float c_d;

    unsigned int triangle_id;

    char pad[4]; // TODO make sure this is ~cache line size
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
    vec3    v0;          // 12
    vec3    v1;          // 12
    vec3    v2;          // 12
    unsigned int triangle_id; //  4  40
#endif

    SetupTriangle();

    SetupTriangle(const Triangle & triangle);

    /**
     * @brief Ray/shape intersection test
     *
     * @param ray    Ray to test against
     * @param result Will be filled with information about the collision
     *
     * @return Whether there was a collision
     */
    bool intersects(const Ray & ray, Collision & result);
};

#endif
