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

#include <math/ray.h>
#include <math/vector.h>
#include <rt_defs.h>
#include <stdlib.h>

// TODO: Moller Trumbore 2D?
// TOOD: Plucker

#define WALD_INTERSECTION
//#define MOLLER_TRUMBORE_INTERSECTION

/**
 * @brief Vertex struct
 */
struct RT_EXPORT Vertex {
    vec3 position; //!< Vertex position
    vec3 normal;   //!< Vertex normal
    vec2 uv;       //!< Vertex UV coordinates

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
     * @param[in] color    Vertex color
     */
    Vertex(
        const vec3 & position,
        const vec3 & normal,
        const vec2 & uv);
};

/**
 * @brief Full triangle struct, with data needed for shading
 */
struct RT_EXPORT Triangle {
    Vertex       v0;           //!< Vertex 0 // TODO: Figure out order
    Vertex       v1;           //!< Vertex 1
    Vertex       v2;           //!< Vertex 2
    vec3         normal;       //!< Face normal
    unsigned int triangle_id;  //!< Triangle ID

    /**
     * @brief Constructor
     */
    Triangle();

    /**
     * @brief Constructor
     */
    Triangle(
        const Vertex & v1,
        const Vertex & v2,
        const Vertex & v3,
        unsigned int   triangle_id);

    /**
     * @brief Interpolate vertex data using barycentric coordinates
     *
     * @param[in] beta  Beta barycentric coordinate
     * @param[in] gamma Gamma barycentric coordinate
     *
     * @return Interpolated vertex data
     */
    Vertex interpolate(float beta, float gamma) const;
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
    vec3    v0;               // 12
    vec3    e1;               // 12
    vec3    e2;               // 12
    unsigned int triangle_id; //  4
                              // 40
#endif
};

/**
 * @brief Utility class for packing triangle data into format required by
 * ray/triangle intersection tests, and for performing ray/triangle
 * intersection tests.
 */
class SetupTriangleBuffer {
public:

    /**
     * @brief Pack triangle data into setup triangle data
     *
     * @param[in] triangles     An array of pointers to unpacked triangles
     * @param[in] num_triangles Number of triangles to pack
     *
     * @return A pointer to an array of packed triangles
     */
    static SetupTriangle *pack(Triangle **triangles, int num_triangles);

    /**
     * @brief Free memory used by an array of packed triangles
     *
     * @param[in] data Triangle data to free
     */
    static void destroy(SetupTriangle *data);

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
    static bool intersects(
        const Ray     &ray,
        SetupTriangle *data,
        int            count,
        bool           anyCollision,
        float          min,
        float          max,
        Collision     &result);
};

inline Vertex::Vertex() {
}

inline Vertex::Vertex(
    const vec3 & position,
    const vec3 & normal,
    const vec2 & uv)
    : position(position),
      normal(normal),
      uv(uv)
{
}

inline Triangle::Triangle() {
}

inline Triangle::Triangle(
    const Vertex & v0,
    const Vertex & v1,
    const Vertex & v2,
    unsigned int   triangle_id)
    : v0(v0),
      v1(v1),
      v2(v2),
      triangle_id(triangle_id)
{
    vec3 b = normalize(v2.position - v0.position);
    vec3 c = normalize(v1.position - v0.position);

    normal = normalize(cross(c, b));
}

inline Vertex Triangle::interpolate(float beta, float gamma) const {
    float alpha = 1.0f - beta - gamma;

    // TODO:
    //    - Face normal vs. intepolated normal
    //    - Position could be retrieved from ray origin + direction * distance instead

    return Vertex(
        v0.position * alpha + v1.position * beta + v2.position * gamma,
        v0.normal   * alpha + v1.normal   * beta + v2.normal   * gamma,
        v0.uv       * alpha + v1.uv       * beta + v2.uv       * gamma);
}

inline void SetupTriangleBuffer::destroy(SetupTriangle *data) {
    if (data)
        free(data);
}

#endif
