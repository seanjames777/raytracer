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
#include <xmmintrin.h>
#include <smmintrin.h>

//#define WALD_INTERSECTION
//#define MOLLER_TRUMBORE_INTERSECTION
#define MOLLER_TRUMBORE_SIMD_INTERSECTION

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

struct RT_EXPORT SetupTriangleBuffer {
#if defined(WALD_INTERSECTION)
	struct SetupTriangle {
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
	} *data;

	const static size_t elemSize = sizeof(SetupTriangle);
	const static size_t elemStep = 1;
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
	struct SetupTriangle {
		vec3    v0;          // 12
		vec3    e1;          // 12
		vec3    e2;          // 12
		unsigned int triangle_id; //  4  40
	} *data;

	const static size_t elemSize = sizeof(SetupTriangle);
	const static size_t elemStep = 1;
#elif defined(MOLLER_TRUMBORE_SIMD_INTERSECTION)
	// TODO: Better to use SOA or just unpack 4 at a time?

	struct SetupTriangle {
		__m128 v0x, v0y, v0z;
		__m128 e1x, e1y, e1z;
		__m128 e2x, e2y, e2z;
		unsigned int triangle_id[4];
	} *data;

	const static size_t elemSize = sizeof(SetupTriangle);
	const static size_t elemStep = 4;
#endif

    SetupTriangleBuffer(Triangle **triangles, int num_triangles);

	~SetupTriangleBuffer();

	bool intersects(const Ray & ray, int count, bool anyCollision, Collision & result);

};

inline Vertex::Vertex() {
}

inline Vertex::Vertex(vec3 position, vec3 normal, vec2 uv, vec4 color)
	: position(position),
	normal(normal),
	uv(uv),
	color(color)
{
}

inline Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3)
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

inline Vertex Triangle::interpolate(float beta, float gamma) {
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

inline SetupTriangleBuffer::SetupTriangleBuffer(Triangle **triangles, int num_triangles) {
#if defined(WALD_INTERSECTION)
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
	b_nv = b.v[u] / denom;
	b_d = (b.v[v] * v1.v[u] - b.v[u] * v1.v[v]) / denom;

	c_nu = c.v[v] / denom;
	c_nv = -c.v[u] / denom;
	c_d = (c.v[u] * v1.v[v] - c.v[v] * v1.v[u]) / denom;
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
	data = new SetupTriangle[num_triangles];

	for (int i = 0; i < num_triangles; i++) {
		const Triangle & tri = *(triangles[i]);
		data[i].v0 = tri.v1.position;
		data[i].e1 = tri.v2.position - tri.v1.position;
		data[i].e2 = tri.v3.position - tri.v1.position;
		data[i].triangle_id = tri.triangle_id;
	}
#elif defined(MOLLER_TRUMBORE_SIMD_INTERSECTION)
	data = new SetupTriangle[(num_triangles + 3) / 4];

	for (int i0 = 0; i0 < num_triangles; i0 += 4) {
		int j = i0 / 4;
		SetupTriangle & setup = data[j];

		for (int k = 0; k < 4; k++) {
			int i = i0 + k;

			if (i >= num_triangles)
				break;

			const Triangle & tri = *(triangles[i]);

			setup.v0x.m128_f32[k] = tri.v1.position.x;
			setup.v0y.m128_f32[k] = tri.v1.position.y;
			setup.v0z.m128_f32[k] = tri.v1.position.z;

			setup.e1x.m128_f32[k] = tri.v2.position.x - tri.v1.position.x;
			setup.e1y.m128_f32[k] = tri.v2.position.y - tri.v1.position.y;
			setup.e1z.m128_f32[k] = tri.v2.position.z - tri.v1.position.z;

			setup.e2x.m128_f32[k] = tri.v3.position.x - tri.v1.position.x;
			setup.e2y.m128_f32[k] = tri.v3.position.y - tri.v1.position.y;
			setup.e2z.m128_f32[k] = tri.v3.position.z - tri.v1.position.z;

			setup.triangle_id[k] = tri.triangle_id;
		}
	}
#endif
}

inline SetupTriangleBuffer::~SetupTriangleBuffer() {
#if defined(WALD_INTERSECTION)
	if (data)
		delete [] data;
#elif defined(MOLLER_TRUMBORE_INTERSECTION)
	if (data)
		delete[] data;
#elif defined(MOLLER_TRUMBORE_SIMD_INTERSECTION)
	if (data)
		delete[] data;
#endif
}

// TODO: Moller Trumbore 2D?
// TOOD: Plucker
inline bool SetupTriangleBuffer::intersects(const Ray & ray, int count, bool anyCollision, Collision & result) {
#if defined(WALD_INTERSECTION)
	bool found = false;

	for (int i = 0; i < count; i++) {
		const SetupTriangle & tri = data[i];

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

		float beta = (hu * b_nu + hv * b_nv + b_d);
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

		if (t < 0.0f || (t >= result.distance && found))
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
#elif defined(MOLLER_TRUMBORE_SIMD_INTERSECTION)
	bool found = false;

	// Constants
	__m128 epsilon_4 = _mm_set1_ps(0.000001f);
	__m128 zero_4 = _mm_set1_ps(0.0f);
	__m128 one_4 = _mm_set1_ps(1.0f);

	// Loaded from ray
	__m128 rox_4 = _mm_set1_ps(ray.origin.x);
	__m128 roy_4 = _mm_set1_ps(ray.origin.y);
	__m128 roz_4 = _mm_set1_ps(ray.origin.z);
	__m128 rdx_4 = _mm_set1_ps(ray.direction.x);
	__m128 rdy_4 = _mm_set1_ps(ray.direction.y);
	__m128 rdz_4 = _mm_set1_ps(ray.direction.z);

	// TODO: __m256
	for (int i0 = 0; i0 < count; i0 += 4) {
		// TODO: Make sure this can use an aligned move--needs to be aligned in memory
		// Loaded from buffer
		int j = i0 / 4;
		__m128 v0x = data[j].v0x;
		__m128 v0y = data[j].v0y;
		__m128 v0z = data[j].v0z;

		__m128 e1x = data[j].e1x;
		__m128 e1y = data[j].e1y;
		__m128 e1z = data[j].e1z;

		__m128 e2x = data[j].e2x;
		__m128 e2y = data[j].e2y;
		__m128 e2z = data[j].e2z;

		// Active triangle mask
		__m128i mask_all = _mm_set1_epi32(0xFFFFFFFF);

		__m128i idx = _mm_setr_epi32(i0, i0 + 1, i0 + 2, i0 + 3);
		__m128i count_4 = _mm_set1_epi32(count);
		__m128i valid = _mm_cmplt_epi32(idx, count_4);

		// TODO: There are FMA opportunities in here, but not on my lame CPU

		// p[i] = cross(ray.direction, e2[i]);
		__m128 px = _mm_sub_ps(_mm_mul_ps(rdy_4, e2z), _mm_mul_ps(rdz_4, e2y));
		__m128 py = _mm_sub_ps(_mm_mul_ps(rdz_4, e2x), _mm_mul_ps(rdx_4, e2z));
		__m128 pz = _mm_sub_ps(_mm_mul_ps(rdx_4, e2y), _mm_mul_ps(rdy_4, e2x));

		// det[i] = dot(e1[i], p[i]);
		__m128 det = _mm_add_ps(_mm_add_ps(_mm_mul_ps(e1x, px), _mm_mul_ps(e1y, py)), _mm_mul_ps(e1z, pz));

		// if (det > epsilon)
		valid = _mm_and_si128(valid, _mm_castps_si128(_mm_cmpgt_ps(det, epsilon_4)));

		// Vote whether to stop
		// TODO: Maybe we can sort somehow to make this more coherent
		if (_mm_test_all_zeros(mask_all, valid))
			continue;

		// f[i] = 1.0f / det[i]
		__m128 f = _mm_div_ps(one_4, det);

		// s[i] = ray.origin - v0[i];
		__m128 sx = _mm_sub_ps(rox_4, v0x);
		__m128 sy = _mm_sub_ps(roy_4, v0y);
		__m128 sz = _mm_sub_ps(roz_4, v0z);

		// beta[i] = f[i] * dot(s[i], p[i]);
		__m128 dsp = _mm_add_ps(_mm_add_ps(_mm_mul_ps(sx, px), _mm_mul_ps(sy, py)), _mm_mul_ps(sz, pz));
		__m128 beta = _mm_mul_ps(f, dsp);

		// if (beta[i] >= 0.0f && beta[i] <= 1.0f)
		valid = _mm_and_si128(valid, _mm_castps_si128(_mm_cmpge_ps(beta, zero_4)));
		valid = _mm_and_si128(valid, _mm_castps_si128(_mm_cmple_ps(beta, one_4)));

		// Vote whether to stop
		// TODO: Maybe worth doing this twice? Doubt it
		if (_mm_test_all_zeros(mask_all, valid))
			continue;

		// q[i] = cross(s[i], e1[i]);
		__m128 qx = _mm_sub_ps(_mm_mul_ps(sy, e1z), _mm_mul_ps(sz, e1y));
		__m128 qy = _mm_sub_ps(_mm_mul_ps(sz, e1x), _mm_mul_ps(sx, e1z));
		__m128 qz = _mm_sub_ps(_mm_mul_ps(sx, e1y), _mm_mul_ps(sy, e1x));

		// gamma[i] = f[i] * dot(ray.direction, q[i]);
		__m128 drdq = _mm_add_ps(_mm_add_ps(_mm_mul_ps(rdx_4, qx), _mm_mul_ps(rdy_4, qy)), _mm_mul_ps(rdz_4, qz));
		__m128 gamma = _mm_mul_ps(f, drdq);

		// if (gamma[i] >= 0.0f && beta[i] + gamma[i] <= 1.0f)
		valid = _mm_and_si128(valid, _mm_castps_si128(_mm_cmpge_ps(gamma, zero_4)));
		valid = _mm_and_si128(valid, _mm_castps_si128(_mm_cmple_ps(_mm_add_ps(beta, gamma), one_4)));

		// Vote whether to stop
		if (_mm_test_all_zeros(mask_all, valid))
			continue;

		// t[i] = f[i] * dot(e2[i], q[i]);
		__m128 de2q = _mm_add_ps(_mm_add_ps(_mm_mul_ps(e2x, qx), _mm_mul_ps(e2y, qy)), _mm_mul_ps(e2z, qz));
		__m128 t = _mm_mul_ps(f, de2q);

		// if (t[i] >= 0.0f)
		valid = _mm_and_si128(valid, _mm_castps_si128(_mm_cmpge_ps(t, zero_4)));

		if (found) {
			__m128  max_4 = _mm_set1_ps(result.distance);
			valid = _mm_and_si128(valid, _mm_castps_si128(_mm_cmplt_ps(t, max_4)));
		}

		// TODO: Could keep four min results and collapse them down at the end instead of every 4
		for (int i = 0; i < 4; i++) {
			if (!valid.m128i_u32[i])
				continue;

			result.distance = t.m128_f32[i];
			result.beta = beta.m128_f32[i];
			result.gamma = gamma.m128_f32[i];
			result.triangle_id = data[j].triangle_id[i];
			found = true;

			if (anyCollision)
				return true;
		}
	}

	return found;
#endif
}


#endif
