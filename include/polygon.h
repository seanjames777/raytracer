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
	#define MM_LOAD_INT       _mm_load_si128
	#define MM_SET1_INT4      _mm_set1_epi32
	#define MM_SETR_INT4      _mm_setr_epi32
	#define MM_CMPGT_INT4     _mm_cmpgt_epi32
	#define MM_SET1_INT       _mm_set1_epi32
	#define MM_ADD_INT4       _mm_add_epi32
	#define MM_AND_PS         _mm_and_ps
	#define MM_ANDNOT_PS      _mm_andnot_ps
	#define MM_SUB_PS         _mm_sub_ps
	#define MM_ADD_PS         _mm_add_ps
	#define MM_MUL_PS         _mm_mul_ps
	#define MM_DIV_PS         _mm_div_ps
	#define MM_OR_PS          _mm_or_ps
	#define MM_NOT_PS         _mm_not_ps
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

namespace SetupTriangleBuffer {
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
	
	const static size_t elemSize = sizeof(SetupTriangle);
	const static size_t elemStep = MM_STEP;
#endif

	char *pack(Triangle **triangles, int num_triangles);

	void destroy(char *data);

	bool intersects(const Ray & ray, char *data, int count, bool anyCollision, Collision & result);
}

#endif
