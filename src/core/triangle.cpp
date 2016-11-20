/**
 * @file core/triangle.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/triangle.h>

#include <string.h>
#include <util/align.h>

/**
 * @brief Pack triangle data into setup triangle data
 *
 * @param[in] triangles     An array of pointers to unpacked triangles
 * @param[in] num_triangles Number of triangles to pack
 */
void setupTriangles(Triangle **triangles, SetupTriangle *data, int num_triangles)
{
#if defined(WALD_INTERSECTION)
    // TODO: should this be aligned (and possibly padded) to a cache line to
    // make sure it only requires one memory request?
    static const int mod_table[5] = { 0, 1, 2, 0, 1 };
    
    for (int i = 0; i < num_triangles; i++) {
        const Triangle & tri = *(triangles[i]);
        SetupTriangle & setup = data[i];
        
        const float3 & v0 = tri.v0.position;
        const float3 & v1 = tri.v1.position;
        const float3 & v2 = tri.v2.position;
        
        // Edges and normal
        float3 b = v2 - v0;
        float3 c = v1 - v0;
        float3 n = cross(c, b);
        
        // Choose which dimension to project
        if (fabs(n.x) > fabs(n.y))
            setup.k = fabs(n.x) > fabs(n.z) ? 0 : 2;
        else
            setup.k = fabs(n.y) > fabs(n.z) ? 1 : 2;
        
        int u = mod_table[setup.k + 1]; // TODO %
        int v = mod_table[setup.k + 2];
        
        n = n / n[setup.k];
        
        setup.n_u = n[u];
        setup.n_v = n[v];
        setup.n_d = dot(v0, n);
        
        // TODO: inv_denom
        
        float denom = b[u] * c[v] - b[v] * c[u];
        setup.b_nu = -b[v] / denom;
        setup.b_nv = b[u] / denom;
        setup.b_d = (b[v] * v0[u] - b[u] * v0[v]) / denom;
        
        setup.c_nu = c[v] / denom;
        setup.c_nv = -c[u] / denom;
        setup.c_d = (c[u] * v0[v] - c[v] * v0[u]) / denom;
        
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
	THREAD Collision     &result)
{
#if defined(WALD_INTERSECTION)
	// http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf
	bool found = false;
	const int mod_table[5] = { 0, 1, 2, 0, 1 };

	for (int i = 0; i < count; i++) {
		GLOBAL const SetupTriangle & tri = data[i];

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