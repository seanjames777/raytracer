/**
 * @file polygon.cpp
 *
 * @author Sean James
 */

// TODO: triangle.cpp?

#include <polygon.h>
#include <string.h>

#include <iostream> // TODO

// TODO: a lot of this can be vectorized

Vertex::Vertex() {
}

Vertex::Vertex(vec3 position, vec3 normal, vec2 uv, vec4 color)
    : position(position),
      normal(normal),
      uv(uv),
      color(color)
{
}

Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3)
    : v1(v1),
      v2(v2),
      v3(v3),
      triangle_id(0) // TODO kinda annoying
{
    bbox = AABB(v1.position, v1.position);
    bbox.join(v2.position);
    bbox.join(v3.position);

    vec3 b = normalize(v3.position - v1.position);
    vec3 c = normalize(v2.position - v1.position);

    normal = normalize(cross(c, b));
}

Vertex Triangle::interpolate(float beta, float gamma) {
    float alpha = 1.0f - beta - gamma;

    // TODO:
    //    - Face normal vs. intepolated normal
    //    - Position could be retrieved from ray origin + direction * distance instead

    return Vertex(
        v1.position * alpha + v2.position * beta + v3.position * gamma,
        v1.normal   * alpha + v2.normal   * beta + v3.normal   * gamma,
        v1.uv       * alpha + v2.uv       * beta + v3.uv       * gamma,
        v1.color    * alpha + v2.color    * beta + v3.color    * gamma);
}

SetupTriangle::SetupTriangle() {
}

SetupTriangle::SetupTriangle(const Triangle & triangle)
    : triangle_id(triangle.triangle_id)
{
#ifdef WALD_INTERSECTION
    // TODO: this should be aligned (and possibly padded) to a cache line to
    // make sure it only requires one memory request.
    static const int mod_table[5] = { 0, 1, 2, 0, 1 };

    const vec3 & v1 = triangle.v1.position;
    const vec3 & v2 = triangle.v2.position;
    const vec3 & v3 = triangle.v3.position;

    // Edges and normal
    vec3 b = v3 - v1;
    vec3 c = v2 - v1;
    vec3 n = cross(c, b);

    // Choose which dimension to project
    if (fabs(n.x) > fabs(n.y))
        k = fabs(n.x) > fabs(n.z) ? 0 : 2;
    else
        k = fabs(n.y) > fabs(n.z) ? 1 : 2;

    int u = mod_table[k + 1]; // TODO %
    int v = mod_table[k + 2];

    n = n / n.v[k];

    n_u = n.v[u];
    n_v = n.v[v];
    n_d = dot(v1, n);

    // TODO: inv_denom

    float denom = b.v[u] * c.v[v] - b.v[v] * c.v[u];
    b_nu = -b.v[v] / denom;
    b_nv =  b.v[u] / denom;
    b_d  =  (b.v[v] * v1.v[u] - b.v[u] * v1.v[v]) / denom;

    c_nu =  c.v[v] / denom;
    c_nv = -c.v[u] / denom;
    c_d  =  (c.v[u] * v1.v[v] - c.v[v] * v1.v[u]) / denom;
#else
	v0 = triangle.v1.position;
	v1 = triangle.v2.position;
	v2 = triangle.v3.position;
#endif
}

bool SetupTriangle::intersects(const Ray & ray, Collision & result) {
#if defined(WALD_INTERSECTION)
    static const int mod_table[5] = { 0, 1, 2, 0, 1 };

    // http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf

    // TODO: lots of branching here.
    // TODO: might be better to *not* precompute stuff for memory bandwidth
    // TODO: inline this?
    // TODO: can early out with max dist
    // TODO: does using result make a difference: register vs. memory?

    int u = mod_table[k + 1];
    int v = mod_table[k + 2];

    float dot = (ray.direction.v[k] + n_u * ray.direction.v[u] + n_v *
        ray.direction.v[v]);

    // TODO: necessary?
    if (dot == 0.0f)
        return false;

    float nd = 1.0f / dot;
    float t_plane = (n_d - ray.origin.v[k]
        - n_u * ray.origin.v[u] - n_v * ray.origin.v[v]) * nd;

    // Behind camera
    if (t_plane <= 0.0f)
        return false;

    float hu = ray.origin.v[u] + t_plane * ray.direction.v[u];
    float hv = ray.origin.v[v] + t_plane * ray.direction.v[v];

    float beta  = (hu * b_nu + hv * b_nv + b_d);
    if (beta < 0.0f)
        return false;

    float gamma = (hu * c_nu + hv * c_nv + c_d);
    if (gamma < 0.0f)
        return false;

    if (beta + gamma > 1.0f)
        return false;

    result.distance = t_plane;
    result.beta = beta;
    result.gamma = gamma;
    result.triangle_id = triangle_id;

    return true;
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;

    vec3 p = cross(ray.direction, e2);

    float det = dot(e1, p);

	// Backfacing or parallel to ray
    if(det <= 0.000001f) // TODO
        return false;

    float f = 1.0f / det;

    vec3 s = ray.origin - v0;
    float beta = f * dot(s, p);

    if(beta < 0.0f || beta > 1.0f)
        return false;

    vec3 q = cross(s, e1);

    float gamma = f * dot(ray.direction, q);

    if(gamma < 0.0f || beta + gamma > 1.0f)
        return false;

    float t = f * dot(e2, q);

    if (t < 0.0f)
        return false;

    result.distance = t;
    result.beta = beta;
    result.gamma = gamma;
    result.triangle_id = triangle_id;

    return true;
#endif

    // TODO: Moller Tumbore 2D
}
