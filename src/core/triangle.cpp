/**
 * @file core/triangle.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/triangle.h>

#include <string.h>
#include <util/align.h>

SetupTriangle *SetupTriangleBuffer::pack(Triangle **triangles, int num_triangles) {
    SetupTriangle *data = (SetupTriangle *)malloc(
        sizeof(SetupTriangle) * num_triangles);

#if defined(WALD_INTERSECTION)
    // TODO: should this be aligned (and possibly padded) to a cache line to
    // make sure it only requires one memory request?
    static const int mod_table[5] = { 0, 1, 2, 0, 1 };

    for (int i = 0; i < num_triangles; i++) {
        const Triangle & tri = *(triangles[i]);
        SetupTriangle & setup = data[i];

        const vec3 & v0 = tri.v0.position;
        const vec3 & v1 = tri.v1.position;
        const vec3 & v2 = tri.v2.position;

        // Edges and normal
        vec3 b = v2 - v0;
        vec3 c = v1 - v0;
        vec3 n = cross(c, b);

        // Choose which dimension to project
        if (fabs(n.x) > fabs(n.y))
            setup.k = fabs(n.x) > fabs(n.z) ? 0 : 2;
        else
            setup.k = fabs(n.y) > fabs(n.z) ? 1 : 2;

        int u = mod_table[setup.k + 1]; // TODO %
        int v = mod_table[setup.k + 2];

        n = n / n.v[setup.k];

        setup.n_u = n.v[u];
        setup.n_v = n.v[v];
        setup.n_d = dot(v0, n);

        // TODO: inv_denom

        float denom = b.v[u] * c.v[v] - b.v[v] * c.v[u];
        setup.b_nu = -b.v[v] / denom;
        setup.b_nv = b.v[u] / denom;
        setup.b_d = (b.v[v] * v0.v[u] - b.v[u] * v0.v[v]) / denom;

        setup.c_nu = c.v[v] / denom;
        setup.c_nv = -c.v[u] / denom;
        setup.c_d = (c.v[u] * v0.v[v] - c.v[v] * v0.v[u]) / denom;

        setup.triangle_id = tri.triangle_id;
    }
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
    for (int i = 0; i < num_triangles; i++) {
        const Triangle & tri = *(triangles[i]);
        SetupTriangle & setup = data[i];

        setup.v0 = tri.v0.position;
        setup.e1 = tri.v1.position - tri.v0.position;
        setup.e2 = tri.v2.position - tri.v0.position;
        setup.triangle_id = tri.triangle_id;
    }
#endif

    return data;
}

bool SetupTriangleBuffer::intersects(
    const Ray     &ray,
    SetupTriangle *data,
    int            count,
    bool           anyCollision,
    float          min,
    float          max,
    Collision     &result)
{
#if defined(WALD_INTERSECTION)
    // http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf
    bool found = false;
    static const int mod_table[5] = { 0, 1, 2, 0, 1 };

    for (int i = 0; i < count; i++) {
        const SetupTriangle & tri = data[i];

        int u = mod_table[tri.k + 1];
        int v = mod_table[tri.k + 2];

        float dot = (ray.direction.v[tri.k] + tri.n_u * ray.direction.v[u] + tri.n_v *
            ray.direction.v[v]);

        // TODO: necessary?
        if (dot == 0.0f)
            continue;

        float nd = 1.0f / dot;
        float t_plane = (tri.n_d - ray.origin.v[tri.k]
            - tri.n_u * ray.origin.v[u] - tri.n_v * ray.origin.v[v]) * nd;

        // Behind camera or further
        if (t_plane <= 0.0f || (found && t_plane >= result.distance) || t_plane < min || t_plane > max)
            continue;

        float hu = ray.origin.v[u] + t_plane * ray.direction.v[u];
        float hv = ray.origin.v[v] + t_plane * ray.direction.v[v];

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
        vec3 p = cross(ray.direction, tri.e2);

        float det = dot(tri.e1, p);

        // Backfacing or parallel to ray
        if (det <= 0.000001f) // TODO
            continue;

        float f = 1.0f / det;

        vec3 s = ray.origin - tri.v0;
        float beta = f * dot(s, p);

        if (beta < 0.0f || beta > 1.0f)
            continue;

        vec3 q = cross(s, tri.e1);

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
