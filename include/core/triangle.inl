#ifndef __TRIANGLE_INL_H
#define __TRIANGLE_INL_H

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
                Ray                    ray,
                GLOBAL SetupTriangle * data,
                int                    count,
                float                  min,
                float                  max,
                THREAD Collision     & result)
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
        if ((found && t_plane >= result.distance) || t_plane < min || t_plane > max)
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

		// TODO: Could break here, but SIMD
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
        
        float3 s = ray.origin - tri.v[0];
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

template<unsigned int N>
vector<bool, N> intersectsPacket(
	const Packet<N>           & packet,
	GLOBAL SetupTriangle      * data,
	int                         count,
	const vector<float, N>    & min,
	const vector<float, N>    & max,
	THREAD PacketCollision<N> & result)
{
 	vector<bool, N> hit = false;

	for (int i = 0; i < N; i++) {
		Ray ray(float3(packet.origin[0][i], packet.origin[1][i], packet.origin[2][i]), float3(packet.direction[0][i], packet.direction[1][i], packet.direction[2][i]));

		Collision _result;
		_result.distance = result.distance[i];

		hit[i] = intersects(ray, data, count, min[i], max[i], _result);

		result.beta[i] = _result.beta;
		result.gamma[i] = _result.gamma;
		result.distance[i] = _result.distance;
		result.triangle_id[i] = _result.triangle_id;
	}

	return hit;
}

template vector<bool, 1> intersectsPacket(
	const Packet<1>           & packet,
	GLOBAL SetupTriangle      * data,
	int                         count,
	const vector<float, 1>    & min,
	const vector<float, 1>    & max,
	THREAD PacketCollision<1> & result);

#endif /* triangle_h */
