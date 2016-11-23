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
	// TODO: go back to early exit version
#if 0
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
#else
	// http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf
	bool found = false;
	const int mod_table[5] = { 0, 1, 2, 0, 1 };

	for (int i = 0; i < count; i++) {
		bool hit = true;

		GLOBAL const SetupTriangle & tri = data[i];

		int u = mod_table[tri.k + 1];
		int v = mod_table[tri.k + 2];

		float dot = (ray.direction[tri.k] + tri.n_u * ray.direction[u] + tri.n_v *
			ray.direction[v]);

		// TODO: necessary?
		hit = hit && (dot != 0.0f);

		float nd = 1.0f / dot;
		float t_plane = (tri.n_d - ray.origin[tri.k]
			- tri.n_u * ray.origin[u] - tri.n_v * ray.origin[v]) * nd;

		// Behind camera or further
		hit = hit && !((found && t_plane >= result.distance) || t_plane < min || t_plane > max);

		float hu = ray.origin[u] + t_plane * ray.direction[u];
		float hv = ray.origin[v] + t_plane * ray.direction[v];

		float beta = (hu * tri.b_nu + hv * tri.b_nv + tri.b_d);
		hit = hit && beta >= 0.0f;

		float gamma = (hu * tri.c_nu + hv * tri.c_nv + tri.c_d);
		hit = hit && gamma >= 0.0f;

		hit = hit && beta + gamma <= 1.0f;

		result.distance = hit ? t_plane : result.distance;
		result.beta = hit ? beta : result.beta;
		result.gamma = hit ? gamma : result.gamma;
		result.triangle_id = hit ? tri.triangle_id : result.triangle_id;

		found = found || hit;

		// TODO: Could break here, but SIMD
}

	return found;
#endif
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
vector<bmask, N> intersectsPacket(
	const Packet<N>           & packet,
	GLOBAL SetupTriangle      * data,
	int                         count,
	const vector<float, N>    & min,
	const vector<float, N>    & max,
	THREAD PacketCollision<N> & result)
{
	// http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf
	vector<bmask, N> found = vector<bmask, N>(0x00000000);
	const int mod_table[5] = { 0, 1, 2, 0, 1 };

	for (int i = 0; i < count; i++) {
		GLOBAL const SetupTriangle & tri = data[i];

		int u = mod_table[tri.k + 1];
		int v = mod_table[tri.k + 2];

		// TODO: Some of these broadcast to 4 channels, which could be done earlier at the cost of
		// bigger triangle data

		// TODO: Can bail early if all of the rays miss a triangle

		vector<float, N> dot = (packet.direction[tri.k] + vector<float, N>(tri.n_u) * packet.direction[u] + vector<float, N>(tri.n_v) *
			packet.direction[v]);

		vector<bmask, N> hit = (dot != vector<float, N>(0.0f));

		vector<float, N> nd = vector<float, N>(1.0f) / dot;

		vector<float, N> t_plane = (vector<float, N>(tri.n_d) - packet.origin[tri.k]
			- vector<float, N>(tri.n_u) * packet.origin[u] - vector<float, N>(tri.n_v) * packet.origin[v]) * nd;

		// Behind camera or further
		hit = hit & ~((found & (t_plane >= result.distance)) | (t_plane < min) | (t_plane > max));

		vector<float, N> hu = packet.origin[u] + t_plane * packet.direction[u];
		vector<float, N> hv = packet.origin[v] + t_plane * packet.direction[v];

		vector<float, N> beta = (hu * vector<float, N>(tri.b_nu) + hv * vector<float, N>(tri.b_nv) + vector<float, N>(tri.b_d));

		hit = hit & (beta >= vector<float, N>(0.0f));

		vector<float, N> gamma = (hu * vector<float, N>(tri.c_nu) + hv * vector<float, N>(tri.c_nv) + vector<float, N>(tri.c_d));

		hit = hit & (gamma >= vector<float, N>(0.0f));

		hit = hit & (beta + gamma <= vector<float, N>(1.0f));

		result.distance = blend(hit, result.distance, t_plane);
		result.beta = blend(hit, result.beta, beta);
		result.gamma = blend(hit, result.gamma, gamma);
		result.triangle_id = blend(hit, result.triangle_id, vector<int, N>(tri.triangle_id)); // TODO: broadcast

		found = found | hit;

		// TODO: Could break here, but SIMD
	}

	return found;
}

template vector<bmask, SIMD> intersectsPacket(
	const Packet<SIMD>           & packet,
	GLOBAL SetupTriangle      * data,
	int                         count,
	const vector<float, SIMD>    & min,
	const vector<float, SIMD>    & max,
	THREAD PacketCollision<SIMD> & result);

#endif /* triangle_h */
