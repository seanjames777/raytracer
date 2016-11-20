/**
 * @file core/triangle.h
 *
 * @brief Triangle, setup triangle, vertex, and triangle buffer classes, and ray/triangle
 * intersection tests.
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __TRIANGLE_H
#define __TRIANGLE_H

#include <rt_defs.h>
#include <math/ray.h>

// TODO: Moller Trumbore 2D?
// TOOD: Plucker

#define WALD_INTERSECTION
//#define MOLLER_TRUMBORE_INTERSECTION

// TODO: Pack these types

/**
 * @brief Vertex struct
 */
struct Vertex {
    float3 position; //!< Vertex position
    float3 normal;   //!< Vertex normal
    float2 uv;       //!< Vertex UV coordinates

    /**
     * @brief Constructor
     */
    Vertex();

    /**
     * @brief Construcotr
     *
     * @param[in] position Vertex position
     * @param[in] normal   Vertex normal
     * @param[in] uv       Vertex UV coordinates
     */
    Vertex(
        float3 position,
        float3 normal,
        float2 uv);
};

/**
 * @brief Full triangle struct, with data needed for shading
 */
struct Triangle {
    Vertex       v[3];
    float3       normal;       //!< Face normal
    unsigned int triangle_id;  //!< Triangle ID
    unsigned int material_id;

    /**
     * @brief Constructor
     */
    Triangle();

    /**
     * @brief Constructor
     */
    Triangle(
        Vertex v0,
		Vertex v1,
		Vertex v2,
        unsigned int triangle_id);

    /**
     * @brief Interpolate vertex data using barycentric coordinates
     *
     * @param[in] beta  Beta barycentric coordinate
     * @param[in] gamma Gamma barycentric coordinate
     *
     * @return Interpolated vertex data
     */
    Vertex interpolate(float beta, float gamma) const GLOBAL;
};

/**
 * @brief Ray/triangle intersection data
 */
struct Collision {
    float        distance;    //!< Distance along ray to collision point
    float        beta;        //!< Beta barycentric coordinate at intersection
    float        gamma;       //!< Gamma barycentric coordinate at intersection
    unsigned int triangle_id; //!< ID of intersected triangle
};

/**
 * @brief Minimal triangle data struct optimized for performance of ray/triangle
 * intersection tests.
 */
struct SetupTriangle {
#if defined(WALD_INTERSECTION)
    float n_u;                //  4 normal.u / normal.k
    float n_v;                //  4 normal.v / normal.h
    float n_d;                //  4 constant of plane equation
    int k;                    //  4 projection axis

    // line equation AC
    float b_nu;               //  4
    float b_nv;               //  4
    float b_d;                //  4

    // line equation AB
    float c_nu;               //  4
    float c_nv;               //  4
    float c_d;                //  4

    unsigned int triangle_id; //  4
                              // 44
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
    float3    v[0];               // 12
    float3    e1;               // 12
    float3    e2;               // 12
    unsigned int triangle_id; //  4
                              // 40
#endif
};

inline Vertex::Vertex() {
}

inline Vertex::Vertex(
    float3 position,
    float3 normal,
    float2 uv)
    : position(position),
      normal(normal),
      uv(uv)
{
}

inline Triangle::Triangle() {
}

inline Triangle::Triangle(
    Vertex v0,
    Vertex v1,
    Vertex v2,
    unsigned int   triangle_id)
	: triangle_id(triangle_id)
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;

    float3 b = normalize(v[2].position - v[0].position);
    float3 c = normalize(v[1].position - v[0].position);

    normal = normalize(cross(c, b));
}

inline Vertex Triangle::interpolate(float beta, float gamma) const GLOBAL {
    float alpha = 1.0f - beta - gamma;

    // TODO:
    //    - Face normal vs. intepolated normal
    //    - Position could be retrieved from ray origin + direction * distance instead

    return Vertex(
        v[0].position * alpha + v[1].position * beta + v[2].position * gamma,
        v[0].normal   * alpha + v[1].normal   * beta + v[2].normal   * gamma,
        v[0].uv       * alpha + v[1].uv       * beta + v[2].uv       * gamma);
}

/**
 * @brief Check for collision between an array of packed triangles and a
 * ray. Returns the closest collision, unless @ref anyCollision is true, in
 * which case returns the first collision (useful for shadow rays).
 *
 * TODO: min and max distance
 *
 * @param[in]  ray          Ray to test against
 * @param[in]  data         Array of packed triangles to test
 * @param[in]  count        Number of triangles to test
 * @param[in]  anyCollision Whether to return the first collision
 * @param[in]  min          Minimum collision distance
 * @param[in]  max          Maximum collision distance
 * @param[out] result       Information about collision, if there was one
 *
 * @return True if there was a collision, or false otherwise
 */
bool intersects(
	Ray     ray,
	GLOBAL SetupTriangle *data,
	int            count,
	bool           anyCollision,
	float          min,
	float          max,
	THREAD Collision     &result);

#if !GPU
/**
 * @brief Pack triangle data into setup triangle data
 *
 * @param[in] triangles     An array of pointers to unpacked triangles
 * @param[in] num_triangles Number of triangles to pack
 */
void setupTriangles(Triangle **triangles, SetupTriangle *data, int num_triangles);
#endif

#endif
