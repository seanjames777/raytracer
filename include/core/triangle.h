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
    Vertex       v0;           //!< Vertex 0 // TODO: Figure out order
    Vertex       v1;           //!< Vertex 1
    Vertex       v2;           //!< Vertex 2
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
        Vertex v1,
        Vertex v2,
        Vertex v3,
        unsigned int triangle_id);

#if GPU
    /**
     * @brief Interpolate vertex data using barycentric coordinates
     *
     * @param[in] beta  Beta barycentric coordinate
     * @param[in] gamma Gamma barycentric coordinate
     *
     * @return Interpolated vertex data
     */
    Vertex interpolate(float beta, float gamma) const device;
#endif
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
    float3    v0;               // 12
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
    : v0(v0),
      v1(v1),
      v2(v2),
      triangle_id(triangle_id)
{
    float3 b = normalize(v2.position - v0.position);
    float3 c = normalize(v1.position - v0.position);

    normal = normalize(cross(c, b));
}

#if GPU
inline Vertex Triangle::interpolate(float beta, float gamma) const device {
    float alpha = 1.0f - beta - gamma;

    // TODO:
    //    - Face normal vs. intepolated normal
    //    - Position could be retrieved from ray origin + direction * distance instead

    return Vertex(
        v0.position * alpha + v1.position * beta + v2.position * gamma,
        v0.normal   * alpha + v1.normal   * beta + v2.normal   * gamma,
        v0.uv       * alpha + v1.uv       * beta + v2.uv       * gamma);
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
    device SetupTriangle *data,
    int            count,
    bool           anyCollision,
    float          min,
    float          max,
    thread Collision     &result)
{
#if defined(WALD_INTERSECTION)
    // http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf
    bool found = false;
    const int mod_table[5] = { 0, 1, 2, 0, 1 };
    
    for (int i = 0; i < count; i++) {
        device const SetupTriangle & tri = data[i];
        
        int u = mod_table[tri.k + 1];
        int v = mod_table[tri.k + 2];
        
        float dot = (ray.direction[tri.k] + tri.n_u * ray.direction[u] + tri.n_v *
                     ray.direction[v]);
        
        // TODO: necessary?
        if (dot == 0.0f)
            continue;
        
        float nd = 1.0f / dot;
        float t_plane = (tri.n_d - ray.origin[tri.k]
                         - tri.n_u * ray.origin[u] - tri.n_v * ray.origin[v]) * nd;
        
        // Behind camera or further
        if (t_plane <= 0.0f || (found && t_plane >= result.distance) || t_plane < min || t_plane > max)
            continue;
        
        float hu = ray.origin[u] + t_plane * ray.direction[u];
        float hv = ray.origin[v] + t_plane * ray.direction[v];
        
        float beta = (hu * tri.b_nu + hv * tri.b_nv + tri.b_d);
        if (beta < 0.0f)
            continue;
        
        float gamma = (hu * tri.c_nu + hv * tri.c_nv + tri.c_d);
        if (gamma < 0.0f)
            continue;
        
        if (beta + gamma > 1.0f)
            continue;
        
        result.distance = t_plane;
        result.beta = beta;
        result.gamma = gamma;
        result.triangle_id = tri.triangle_id;
        found = true;
        
        if (anyCollision)
            return true;
    }
    
    return found;
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
    bool found = false;
    
    for (int i = 0; i < count; i++) {
        const SetupTriangle & tri = data[i];
        float3 p = cross(ray.direction, tri.e2);
        
        float det = dot(tri.e1, p);
        
        // Backfacing or parallel to ray
        if (det <= 0.000001f) // TODO
            continue;
        
        float f = 1.0f / det;
        
        float3 s = ray.origin - tri.v0;
        float beta = f * dot(s, p);
        
        if (beta < 0.0f || beta > 1.0f)
            continue;
        
        float3 q = cross(s, tri.e1);
        
        float gamma = f * dot(ray.direction, q);
        
        if (gamma < 0.0f || beta + gamma > 1.0f)
            continue;
        
        float t = f * dot(tri.e2, q);
        
        if (t < 0.0f || (found && t >= result.distance) || t < min || t > max)
            continue;
        
        result.distance = t;
        result.beta = beta;
        result.gamma = gamma;
        result.triangle_id = tri.triangle_id;
        found = true;
        
        if (anyCollision)
            return true;
    }
    
    return found;
#endif
}
#endif

#if CPU
/**
 * @brief Pack triangle data into setup triangle data
 *
 * @param[in] triangles     An array of pointers to unpacked triangles
 * @param[in] num_triangles Number of triangles to pack
 */
void setupTriangles(Triangle **triangles, SetupTriangle *data, int num_triangles);
#endif

#endif
