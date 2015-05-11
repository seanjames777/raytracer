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

#define BUFFER_ALIGN 32
#define ALIGN_PTR(p) (((unsigned long long)(p) + BUFFER_ALIGN - 1) & ~(BUFFER_ALIGN - 1))

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

#define MM_STEP           4
#if MM_STEP == 8
	#define MMVEC             __m256
	#define MMVECI            __m256i
	#define MMVECI4           __m128i
	#define MM_LOAD_PS        _mm256_load_ps
	#define MM_SET1_INT4      _mm_set1_epi32
	#define MM_SETR_INT4      _mm_setr_epi32
	#define MM_CMPGT_INT4     _mm_cmpgt_epi32
	#define MM_SET1_INT       _mm256_set1_epi32
	#define MM_AND_PS         _mm256_and_ps
	#define MM_SUB_PS         _mm256_sub_ps
	#define MM_ADD_PS         _mm256_add_ps
	#define MM_MUL_PS         _mm256_mul_ps
	#define MM_DIV_PS         _mm256_div_ps
	#define MM_SET1_PS        _mm256_set1_ps
	#define MM_CMPGT_PS(a, b) _mm256_cmp_ps(a, b, 30)
	#define MM_CMPGE_PS(a, b) _mm256_cmp_ps(a, b, 29)
	#define MM_CMPLT_PS(a, b) _mm256_cmp_ps(a, b, 17)
	#define MM_CMPLE_PS(a, b) _mm256_cmp_ps(a, b, 18)
	#define MM_CAST_INT_FLOAT _mm256_castps_si256
	#define MM_CAST_FLOAT_INT _mm256_castsi256_ps
	#define MM_ALL_ZERO       _mm256_testz_ps
	#define MM_PI_SUB(v, i)   ((v).m256i_u32[i])
	#define MM_PS_SUB(v, i)   ((v).m256_f32[i])
#elif MM_STEP == 4
	#define MMVEC             __m128
	#define MMVECI            __m128i
	#define MMVECI4           __m128i
	#define MM_LOAD_PS        _mm_load_ps
	#define MM_SET1_INT4      _mm_set1_epi32
	#define MM_SETR_INT4      _mm_setr_epi32
	#define MM_CMPGT_INT4     _mm_cmpgt_epi32
	#define MM_SET1_INT       _mm_set1_epi32
	#define MM_ADD_INT4       _mm_add_epi32
	#define MM_AND_PS         _mm_and_ps
	#define MM_SUB_PS         _mm_sub_ps
	#define MM_ADD_PS         _mm_add_ps
	#define MM_MUL_PS         _mm_mul_ps
	#define MM_DIV_PS         _mm_div_ps
	#define MM_SET1_PS        _mm_set1_ps
	#define MM_CMPGT_PS(a, b) _mm_cmp_ps(a, b, 30)
	#define MM_CMPGE_PS(a, b) _mm_cmp_ps(a, b, 29)
	#define MM_CMPLT_PS(a, b) _mm_cmp_ps(a, b, 17)
	#define MM_CMPLE_PS(a, b) _mm_cmp_ps(a, b, 18)
	#define MM_CAST_INT_FLOAT _mm_castps_si128
	#define MM_CAST_FLOAT_INT _mm_castsi128_ps
	#define MM_ALL_ZERO       _mm_testz_ps
	#define MM_PI_SUB(v, i)   ((v).m128i_u32[i])
	#define MM_PS_SUB(v, i)   ((v).m128_f32[i])
#endif

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
		float v0x[MM_STEP], v0y[MM_STEP], v0z[MM_STEP];
		float e1x[MM_STEP], e1y[MM_STEP], e1z[MM_STEP];
		float e2x[MM_STEP], e2y[MM_STEP], e2z[MM_STEP];
		unsigned int triangle_id[MM_STEP];
	};
	
	char *data; // Triangles are aligned

	const static size_t elemSize = sizeof(SetupTriangle);
	const static size_t elemStep = MM_STEP;
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
	size_t data_size = ((num_triangles + MM_STEP - 1) / MM_STEP) * sizeof(SetupTriangle);

	data = (char *)malloc(data_size + BUFFER_ALIGN);

	SetupTriangle *aligned = (SetupTriangle *)ALIGN_PTR(data);

	for (int i0 = 0; i0 < num_triangles; i0 += MM_STEP) {
		int j = i0 / MM_STEP;
		SetupTriangle & setup = aligned[j];

		for (int k = 0; k < MM_STEP; k++) {
			int i = i0 + k;

			if (i >= num_triangles)
				break;

			const Triangle & tri = *(triangles[i]);

			setup.v0x[k] = tri.v1.position.x;
			setup.v0y[k] = tri.v1.position.y;
			setup.v0z[k] = tri.v1.position.z;

			setup.e1x[k] = tri.v2.position.x - tri.v1.position.x;
			setup.e1y[k] = tri.v2.position.y - tri.v1.position.y;
			setup.e1z[k] = tri.v2.position.z - tri.v1.position.z;
			
			setup.e2x[k] = tri.v3.position.x - tri.v1.position.x;
			setup.e2y[k] = tri.v3.position.y - tri.v1.position.y;
			setup.e2z[k] = tri.v3.position.z - tri.v1.position.z;

			setup.triangle_id[k] = tri.triangle_id;
		}
	}
#endif
}

inline SetupTriangleBuffer::~SetupTriangleBuffer() {
	if (data)
		free(data);
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
	float min_distance = 0.0f;
	float min_gamma = 0.0f;
	float min_beta = 0.0f;
	unsigned int min_id = 0;

	// Constants
	MMVEC epsilon = MM_SET1_PS(0.000001f);
	MMVEC zero = MM_SET1_PS(0.0f);
	MMVEC one = MM_SET1_PS(1.0f);

	// Loaded from ray
	MMVEC rox = MM_SET1_PS(ray.origin.x);
	MMVEC roy = MM_SET1_PS(ray.origin.y);
	MMVEC roz = MM_SET1_PS(ray.origin.z);
	MMVEC rdx = MM_SET1_PS(ray.direction.x);
	MMVEC rdy = MM_SET1_PS(ray.direction.y);
	MMVEC rdz = MM_SET1_PS(ray.direction.z);

	MMVEC mask_all = MM_CAST_FLOAT_INT(MM_SET1_INT(0xFFFFFFFF));
	MMVECI4 idx0 = MM_SETR_INT4(0, 1, 2, 3);
	MMVECI4 idx1 = MM_SETR_INT4(4, 5, 6, 7);
	MMVECI4 incr = MM_SET1_INT4(MM_STEP);

	SetupTriangle *aligned = (SetupTriangle *)ALIGN_PTR(data);

    // TODO: This function doesn't go much faster, probably? because the early outs
    // are divergent? It might be possible to reshuffle triangles that reach each
    // stage or something?

	// TODO: __m256
	for (int i0 = 0; i0 < count; i0 += MM_STEP) {
		// TODO: Make sure these can use an aligned move--needs to be aligned in memory
		// Loaded from buffer
		int j = i0 / MM_STEP;

		MMVEC v0x = MM_LOAD_PS(aligned[j].v0x);
		MMVEC v0y = MM_LOAD_PS(aligned[j].v0y);
		MMVEC v0z = MM_LOAD_PS(aligned[j].v0z);

		MMVEC e1x = MM_LOAD_PS(aligned[j].e1x);
		MMVEC e1y = MM_LOAD_PS(aligned[j].e1y);
		MMVEC e1z = MM_LOAD_PS(aligned[j].e1z);

		MMVEC e2x = MM_LOAD_PS(aligned[j].e2x);
		MMVEC e2y = MM_LOAD_PS(aligned[j].e2y);
		MMVEC e2z = MM_LOAD_PS(aligned[j].e2z);

		// Active triangle mask
		MMVECI4 count4 = MM_SET1_INT4(count);
#if MM_STEP == 4
		MMVEC valid = MM_CAST_FLOAT_INT(MM_CMPGT_INT4(count4, idx0));
		idx0 = MM_ADD_INT4(idx0, incr);
#elif MM_STEP == 8
		MMVECI4 valid0 = MM_CMPGT_INT4(count4, idx0);
		MMVECI4 valid1 = MM_CMPGT_INT4(count4, idx1);
		MMVEC valid = MM_CAST_FLOAT_INT(_mm256_set_m128i(valid1, valid0));
		idx0 = MM_ADD_INT4(idx0, incr);
		idx1 = MM_ADD_INT4(idx1, incr);
#endif

		// TODO: There are FMA opportunities in here, but not on my lame CPU

		// p[i] = cross(ray.direction, e2[i]);
		MMVEC px = MM_SUB_PS(MM_MUL_PS(rdy, e2z), MM_MUL_PS(rdz, e2y));
		MMVEC py = MM_SUB_PS(MM_MUL_PS(rdz, e2x), MM_MUL_PS(rdx, e2z));
		MMVEC pz = MM_SUB_PS(MM_MUL_PS(rdx, e2y), MM_MUL_PS(rdy, e2x));

		// det[i] = dot(e1[i], p[i]);
		MMVEC det = MM_ADD_PS(MM_ADD_PS(MM_MUL_PS(e1x, px), MM_MUL_PS(e1y, py)), MM_MUL_PS(e1z, pz));

		// if (det > epsilon)
		valid = MM_AND_PS(valid, MM_CMPGT_PS(det, epsilon));

		// Vote whether to stop
		// TODO: Maybe we can sort somehow to make this more coherent
		if (MM_ALL_ZERO(mask_all, valid))
			continue;

		// f[i] = 1.0f / det[i]
		MMVEC f = MM_DIV_PS(one, det);

		// s[i] = ray.origin - v0[i];
		MMVEC sx = MM_SUB_PS(rox, v0x);
		MMVEC sy = MM_SUB_PS(roy, v0y);
		MMVEC sz = MM_SUB_PS(roz, v0z);

		// beta[i] = f[i] * dot(s[i], p[i]);
		MMVEC dsp = MM_ADD_PS(MM_ADD_PS(MM_MUL_PS(sx, px), MM_MUL_PS(sy, py)), MM_MUL_PS(sz, pz));
		MMVEC beta = MM_MUL_PS(f, dsp);

		// if (beta[i] >= 0.0f && beta[i] <= 1.0f)
		valid = MM_AND_PS(valid, MM_CMPGE_PS(beta, zero));
		valid = MM_AND_PS(valid, MM_CMPLE_PS(beta, one));

		// Vote whether to stop
		// TODO: Maybe worth doing this twice? Doubt it
		if (MM_ALL_ZERO(mask_all, valid))
			continue;

		// q[i] = cross(s[i], e1[i]);
		MMVEC qx = MM_SUB_PS(MM_MUL_PS(sy, e1z), MM_MUL_PS(sz, e1y));
		MMVEC qy = MM_SUB_PS(MM_MUL_PS(sz, e1x), MM_MUL_PS(sx, e1z));
		MMVEC qz = MM_SUB_PS(MM_MUL_PS(sx, e1y), MM_MUL_PS(sy, e1x));

		// gamma[i] = f[i] * dot(ray.direction, q[i]);
		MMVEC drdq = MM_ADD_PS(MM_ADD_PS(MM_MUL_PS(rdx, qx), MM_MUL_PS(rdy, qy)), MM_MUL_PS(rdz, qz));
		MMVEC gamma = MM_MUL_PS(f, drdq);

		// if (gamma[i] >= 0.0f && beta[i] + gamma[i] <= 1.0f)
		valid = MM_AND_PS(valid, MM_CMPGE_PS(gamma, zero));
		valid = MM_AND_PS(valid, MM_CMPLE_PS(MM_ADD_PS(beta, gamma), one));

		// Vote whether to stop
		if (MM_ALL_ZERO(mask_all, valid))
			continue;

		// t[i] = f[i] * dot(e2[i], q[i]);
		MMVEC de2q = MM_ADD_PS(MM_ADD_PS(MM_MUL_PS(e2x, qx), MM_MUL_PS(e2y, qy)), MM_MUL_PS(e2z, qz));
		MMVEC t = MM_MUL_PS(f, de2q);

		// if (t[i] >= 0.0f)
		valid = MM_AND_PS(valid, MM_CMPGE_PS(t, zero));

		// Vote whether to stop
		if (MM_ALL_ZERO(mask_all, valid))
			continue;

		// TODO: Could keep four min results and collapse them down at the end instead of every 4
		// TODO: Bit scan to find valid indices
		for (int i = 0; i < MM_STEP; i++) {
			if (!MM_PS_SUB(valid, i))
				continue;

			float distance = MM_PS_SUB(t, i);

			if (distance < min_distance || !found) {
				min_distance = distance;
				min_beta = MM_PS_SUB(beta, i);
				min_gamma = MM_PS_SUB(gamma, i);
				min_id = aligned[j].triangle_id[i];
				found = true;
			}

			if (anyCollision)
				goto done;
		}
	}

done:

	if (found) {
		result.distance = min_distance;
		result.beta = min_beta;
		result.gamma = min_gamma;
		result.triangle_id = min_id;
	}

	return found;
#endif
}


#endif
