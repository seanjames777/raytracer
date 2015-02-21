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

//#define USE_SIMD_INTERSECTION

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec4 color;

    Vertex();

    Vertex(vec3 position, vec3 normal, vec2 uv, vec4 color);
};

struct Triangle {
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

class VertexBuffer {
private:

    Vertex       *vertices;
    unsigned int *indices;
    unsigned int  num_vertices;
    unsigned int  num_indices;

public:

    VertexBuffer(
        Vertex       *vertices,
        unsigned int *indices,
        unsigned int  num_vertices,
        unsigned int  num_indices);

    ~VertexBuffer();

};

struct Collision {
    float distance;
    float beta;
    float gamma;
    unsigned int triangle_id;
};

struct SetupTriangle {
#ifndef USE_SIMD_INTERSECTION
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

    char pad[4];
#else
    vec3 a;
    vec3 v0;
    vec3 v1;
    vec3 n;
    float d00;
    float d01;
    float d11;
    float invDenom;

    unsigned int triangle_id;
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
