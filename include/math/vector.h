/**
 * @file math/vector.h
 *
 * @brief Vector type
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_VECTOR_H
#define __MATH_VECTOR_H

#if GPU
#include <simd/simd.h>

using namespace simd;
#else
#include <rt_defs.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <immintrin.h>

using std::min;
using std::max;
using std::swap;

// TODO: move me
template<typename T>
inline T clamp(T val, T a, T b) {
	return min(max(val, min), max);
}

template<typename T>
inline T saturate(T val) {
	return min(max(val, 0.0f), 1.0f);
}

template<typename T, unsigned int N>
struct ALIGN(16) vector {
    T _v[N];
    
    vector<T, N>()
        : _v()
    {
    }

	vector<T, N>(T v){
		for (unsigned int i = 0; i < N; i++)
			_v[i] = v;
	}
    
    T & operator[](unsigned int i) {
        return _v[i];
    }
    
    const T& operator[](unsigned int i) const {
        return _v[i];
    }
};

typedef vector<float, 2> float2;
typedef vector<float, 3> float3;
typedef vector<float, 4> float4;

typedef vector<int, 2> int2;
typedef vector<int, 3> int3;
typedef vector<int, 4> int4;

typedef unsigned int bmask;

typedef vector<bmask, 2> bmask2;
typedef vector<bmask, 3> bmask3;
typedef vector<bmask, 4> bmask4;

#if 0
template<typename T>
struct vector<T, 1> {
	union {
		T _v[1];
		struct {
			T x;
		};
	};

	vector<T, 1>()
		: _v()
	{
	}

	vector<T, 1>(T v)
		: x(v)
	{
	}

	T & operator[](unsigned int i) {
		return _v[i];
	}

	const T& operator[](unsigned int i) const {
		return _v[i];
	}
};

template<typename T>
struct vector<T, 2> {
    union {
        T _v[2];
        struct {
            T x;
            T y;
        };
    };
    
    vector<T, 2>()
        : _v()
    {
    }

    vector<T, 2>(T v)
        : x(v),
          y(v)
    {
    }
    
    vector<T, 2>(T x, T y)
        : x(x),
          y(y)
    {
    }
    
    T & operator[](unsigned int i) {
        return _v[i];
    }
    
    const T& operator[](unsigned int i) const {
        return _v[i];
    }
};

template<typename T>
struct vector<T, 3> {
    union {
        T _v[3];
        struct {
            T x;
            T y;
            T z;
        };
    };
    
    vector<T, 3>()
        : _v()
    {
    }

    vector<T, 3>(T v)
        : x(v),
          y(v),
          z(v)
    {
    }

	vector<T, 3>(const vector<T, 2> & xy, T z)
		: x(xy.x),
		  y(xy.y),
		  z(z)
	{
	}
    
    vector<T, 3>(T x, T y, T z)
        : x(x),
          y(y),
          z(z)
    {
    }
    
    T & operator[](unsigned int i) {
        return _v[i];
    }
    
    const T& operator[](unsigned int i) const {
        return _v[i];
    }
};

template<typename T>
struct vector<T, 4> {
    union {
        T _v[4];
        struct {
            T x;
            T y;
            T z;
            T w;
        };
    };
    
    vector<T, 4>()
        : _v()
    {
    }

    vector<T, 4>(T v)
        : x(v),
          y(v),
          z(v),
          w(v)
    {
    }

	vector<T, 4>(const vector<T, 3> & xyz, T w)
		: x(xyz.x),
	 	  y(xyz.y),
          z(xyz.z),
          w(w)
	{
	}
    
    vector<T, 4>(T x, T y, T z, T w)
        : x(x),
          y(y),
          z(z),
          w(w)
    {
    }

	vector<T, 3> xyz() {
		return vector<T, 3>(x, y, z);
	}
    
    T & operator[](unsigned int i) {
        return _v[i];
    }
    
    const T& operator[](unsigned int i) const {
        return _v[i];
    }
};
#endif

#if 0
template<typename T, unsigned int N>
inline vector<T, N> min(const vector<T, N> & lhs, const vector<T, N> & rhs) {
    vector<T, N> out;
    
    for (unsigned int i = 0; i < N; i++)
		out[i] = min(lhs[i], rhs[i]);
    
    return out;
}

template<typename T, unsigned int N>
inline vector<T, N> max(const vector<T, N> & lhs, const vector<T, N> & rhs) {
    vector<T, N> out;
    
    for (unsigned int i = 0; i < N; i++)
        out[i] = max(lhs[i], rhs[i]);
    
    return out;
}

template<typename T, unsigned int N>
inline vector<T, N> abs(const vector<T, N> & v) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = abs(v[i]);

	return out;
}

template<typename T, unsigned int N>
inline vector<T, N> saturate(const vector<T, N> & v) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = saturate(v[i]);

	return out;
}

template<typename T, unsigned int N>
inline vector<T, N> & operator+=(vector<T, N> & lhs, const vector<T, N> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs[i] += rhs[i];
    return lhs;
}

template<typename T, unsigned int N>
inline vector<T, N> operator+(const vector<T, N> & lhs, const vector<T, N> & rhs) {
    vector<T, N> val = lhs;
    val += rhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> & operator-=(vector<T, N> & lhs, const vector<T, N> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs[i] -= rhs[i];
    return lhs;
}

template<typename T, unsigned int N>
inline vector<T, N> operator-(const vector<T, N> & lhs, const vector<T, N> & rhs) {
    vector<T, N> val = lhs;
    val -= rhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> & operator*=(vector<T, N> & lhs, const vector<T, N> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs[i] *= rhs[i];
    return lhs;
}

template<typename T, unsigned int N>
inline vector<T, N> operator*(const vector<T, N> & lhs, const vector<T, N> & rhs) {
    vector<T, N> val = lhs;
    val *= rhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> & operator/=(vector<T, N> & lhs, const vector<T, N> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs[i] /= rhs[i];
    return lhs;
}

template<typename T, unsigned int N>
inline vector<T, N> operator/(const vector<T, N> & lhs, const vector<T, N> & rhs) {
    vector<T, N> val = lhs;
    val /= rhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> & operator+=(vector<T, N> & lhs, T rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs[i] += rhs;
    return lhs;
}

// TODO: Maybe there's a way to load constants outside the functions in case
// they're reused?

template<typename T, unsigned int N>
inline vector<T, N> operator+(const vector<T, N> & lhs, T rhs) {
    vector<T, N> val = lhs;
    val += rhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> operator+(T lhs, const vector<T, N> & rhs) {
    vector<T, N> val = rhs;
    val += lhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> & operator-=(vector<T, N> & lhs, T rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs[i] -= rhs;
    return lhs;
}

template<typename T, unsigned int N>
inline vector<T, N> operator-(const vector<T, N> & lhs, T rhs) {
    vector<T, N> val = lhs;
    val -= rhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> operator-(T lhs, const vector<T, N> & rhs) {
    vector<T, N> val = rhs;
    val -= lhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> & operator*=(vector<T, N> & lhs, T rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs[i] *= rhs;
    return lhs;
}

template<typename T, unsigned int N>
inline vector<T, N> operator*(const vector<T, N> & lhs, T rhs) {
    vector<T, N> val = lhs;
    val *= rhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> operator*(T lhs, const vector<T, N> & rhs) {
    vector<T, N> val = rhs;
    val *= lhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> & operator/=(vector<T, N> & lhs, T rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs[i] /= rhs;
    return lhs;
}

template<typename T, unsigned int N>
inline vector<T, N> operator/(const vector<T, N> & lhs, T rhs) {
    vector<T, N> val = lhs;
    val /= rhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> operator/(T lhs, const vector<T, N> & rhs) {
    vector<T, N> val = rhs;
    val /= lhs;
    return val;
}

template<typename T, unsigned int N>
inline vector<T, N> operator-(const vector<T, N> & rhs) {
    vector<T, N> val;

    for (unsigned int i = 0; i < N; i++)
        val[i] = -rhs[i];

    return val;
}

template<typename T, unsigned int N>
inline T any(const vector<T, N> & v) {
	for (unsigned int i = 0; i < N; i++)
		if (v[i])
			return true;

	return false;
}

template<typename T, unsigned int N>
inline T all(const vector<T, N> & v) {
	for (unsigned int i = 0; i < N; i++)
		if (!v[i])
			return false;

	return true;
}

template<typename T, typename B, unsigned int N>
inline vector<T, N> blend(const vector<B, N> & mask, const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = mask[i] ? rhs[i] : lhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator<(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] < rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator<=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] <= rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator==(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] == rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator>=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] >= rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator>(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] > rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator!=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] != rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator&&(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] && rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator||(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] || rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator!(const vector<T, N> & v) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = !v[i];

	return out;
}

template<typename T, unsigned int N>
inline T dot(const vector<T, N> & lhs, const vector<T, N> & rhs) {
    T result = 0;

    for (unsigned int i = 0; i < N; i++)
        result += lhs[i] * rhs[i];

    return result;
}

template<typename T, unsigned int N>
inline T length2(const vector<T, N> & vec) {
    return dot(vec, vec);
}

template<typename T, unsigned int N>
inline T length(const vector<T, N> & vec) {
    return static_cast<T>(sqrt(dot(vec, vec)));
}

template<unsigned int N>
inline float length(const vector<float, N> & vec) {
    return sqrtf(dot(vec, vec));
}

template<typename T, unsigned int N>
inline vector<T, N> normalize(const vector<T, N> & v) {
    T len = length(v);
    return v / len;
}

template<typename T>
inline vector<T, 3> cross(const vector<T, 3> & lhs, const vector<T, 3> & rhs) {
    return vector<T, 3>(
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x);
}

/**
 * Reflect this vector across a normal vector. TODO.
 */
template<typename T>
inline vector<T, 3> reflect(const vector<T, 3> & vec, const vector<T, 3> & norm) {
    return 2.0f * dot(vec, norm) * norm - vec;
}

/**
 * Get a refracted vector according to Snell's law. Assumes that this vector points towards
 * the interface. TODO.
 *
 * @param norm Normal at interface
 * @param n1   Index of refraction of material being left
 * @param n2   Index of refractio nof material being entered
 */
template<typename T>
inline vector<T, 3> refract(const vector<T, 3> & vec, const vector<T, 3> & norm, float n1, float n2) {
    vector<T, 3> L = -vec;
    vector<T, 3> N = norm;

    float r = n1 / n2;
    float cos_theta_l = dot(L, N);
    float c = sqrtf(1.0f - r * r * (1.0f - cos_theta_l * cos_theta_l));

    float c_l = -r;
    float c_n = -c - r * cos_theta_l;

    return L * c_l + N * c_n;
}

/**
 * @brief Schlick's approximation. TODO.
 *
 * @param n  Normal
 * @param v  View direction
 * @param n1 Index of refraction of material being left
 * @param n2 Index of refraction of material being entered
 *
 * @return A factor for blending reflection and refraction. 0 = refraction only,
 * 1 = reflection only
 */
template<typename T>
inline float schlick(const vector<T, 3> & n, const vector<T, 3> & v, float n1, float n2) {
    // TODO specular highlights can use this too?

    float cos_i = dot(n, v);

    float r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;

    // TODO: total internal reflection

    return r0 + (1.0f - r0) * pow(1.0f - cos_i, 5.0f);
}
#endif

template<>
struct ALIGN(16) vector<float, 2> {
	union {
		float _v[4];
		struct {
			float x;
			float y;
			float _pad1;
			float _pad2;
		};
		__m128 _s;
	};

	vector()
		: _s(_mm_setzero_ps())
	{
	}

	vector(float v)
		: _s(_mm_broadcast_ss(&v))
	{
	}

	vector(float x, float y)
		: _s(_mm_setr_ps(x, y, 0.0f, 0.0f))
	{
	}

	vector(__m128 s)
		: _s(s)
	{
	}

	float & operator[](unsigned int i) {
		return _v[i];
	}

	const float & operator[](unsigned int i) const {
		return _v[i];
	}
};

template<unsigned int N, unsigned int X, unsigned int Y, unsigned int Z, unsigned int W, unsigned int M>
inline vector<float, N> shuffle(const vector<float, M> v) {
	return _mm_shuffle_ps(v._s, v._s, _MM_SHUFFLE(X, Y, Z, W));
}

inline vector<float, 2> min(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_min_ps(lhs._s, rhs._s);
}

inline vector<float, 2> max(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_max_ps(lhs._s, rhs._s);
}

// TODO: abs
// TODO: saturate. Note: SSE has special functions for saturation

inline vector<float, 2> operator+(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_add_ps(lhs._s, rhs._s);
}

inline vector<float, 2> operator-(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_sub_ps(lhs._s, rhs._s);
}

inline vector<float, 2> operator-(const vector<float, 2> & vec) {
	return _mm_sub_ps(_mm_set1_ps(0.0f), vec._s);
}

inline vector<float, 2> operator*(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_mul_ps(lhs._s, rhs._s);
}

inline vector<float, 2> operator/(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_div_ps(lhs._s, rhs._s);
}

inline float dot(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return vector<float, 2>(_mm_dp_ps(lhs._s, rhs._s, 0x3F)).x;
}

inline float length(const vector<float, 2> & vec) {
	return vector<float, 2>(_mm_sqrt_ps(_mm_dp_ps(vec._s, vec._s, 0x3F))).x;
}

inline vector<float, 2> normalize(const vector<float, 2> & vec) {
	return _mm_div_ps(vec._s, _mm_sqrt_ps(_mm_dp_ps(vec._s, vec._s, 0x3F)));
}

template<>
struct ALIGN(16) vector<float, 3> {
	union {
		float _v[4];
		struct {
			float x;
			float y;
			float z;
			float _pad;
		};
		__m128 _s;
	};

	vector()
		: _s(_mm_setzero_ps())
	{
	}

	vector(float v)
		: _s(_mm_broadcast_ss(&v))
	{
	}

#if 0
	vector(const vector<float, 2> & xy, float z)
		: _s(_mm_setr_ps(xy.x, xy.y, z, 0.0f)) // TODO
	{
	}
#endif

	vector(float x, float y, float z)
		: _s(_mm_setr_ps(x, y, z, 0.0f))
	{
	}

	vector(__m128 s)
		: _s(s)
	{
	}

	float & operator[](unsigned int i) {
		return _v[i];
	}

	const float & operator[](unsigned int i) const {
		return _v[i];
	}
};

inline vector<float, 3> min(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return _mm_min_ps(lhs._s, rhs._s);
}

inline vector<float, 3> max(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return _mm_max_ps(lhs._s, rhs._s);
}

// TODO: abs
// TODO: saturate. Note: SSE has special functions for saturation

inline vector<float, 3> operator+(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return _mm_add_ps(lhs._s, rhs._s);
}

inline vector<float, 3> operator-(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return _mm_sub_ps(lhs._s, rhs._s);
}

inline vector<float, 3> operator-(const vector<float, 3> & vec) {
	return _mm_sub_ps(_mm_set1_ps(0.0f), vec._s);
}

inline vector<float, 3> operator*(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return _mm_mul_ps(lhs._s, rhs._s);
}

inline vector<float, 3> operator/(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return _mm_div_ps(lhs._s, rhs._s);
}

inline float dot(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return vector<float, 3>(_mm_dp_ps(lhs._s, rhs._s, 0x71)).x;
}

inline float length(const vector<float, 3> & vec) {
	return vector<float, 3>(_mm_sqrt_ps(_mm_dp_ps(vec._s, vec._s, 0x7F))).x;
}

inline vector<float, 3> normalize(const vector<float, 3> & vec) {
	// TODO: Check if rsqrt is accurate enough and faster
	return _mm_div_ps(vec._s, _mm_sqrt_ps(_mm_dp_ps(vec._s, vec._s, 0x7F)));
}

// TODO: These can probably be implemented more efficiently
inline vector<float, 3> cross(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return vector<float, 3>(
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x);
}

/**
* Reflect this vector across a normal vector. TODO.
*/
inline vector<float, 3> reflect(const vector<float, 3> & vec, const vector<float, 3> & norm) {
	return 2.0f * dot(vec, norm) * norm - vec;
}

/**
* Get a refracted vector according to Snell's law. Assumes that this vector points towards
* the interface. TODO.
*
* @param norm Normal at interface
* @param n1   Index of refraction of material being left
* @param n2   Index of refractio nof material being entered
*/
inline vector<float, 3> refract(const vector<float, 3> & vec, const vector<float, 3> & norm, float n1, float n2) {
	vector<float, 3> L = -vec;
	vector<float, 3> N = norm;

	float r = n1 / n2;
	float cos_theta_l = dot(L, N);
	float c = sqrtf(1.0f - r * r * (1.0f - cos_theta_l * cos_theta_l));

	float c_l = -r;
	float c_n = -c - r * cos_theta_l;

	return L * c_l + N * c_n;
}

/**
* @brief Schlick's approximation. TODO.
*
* @param n  Normal
* @param v  View direction
* @param n1 Index of refraction of material being left
* @param n2 Index of refraction of material being entered
*
* @return A factor for blending reflection and refraction. 0 = refraction only,
* 1 = reflection only
*/
inline float schlick(const vector<float, 3> & n, const vector<float, 3> & v, float n1, float n2) {
	// TODO specular highlights can use this too?

	float cos_i = dot(n, v);

	float r0 = (n1 - n2) / (n1 + n2);
	r0 *= r0;

	// TODO: total internal reflection

	return r0 + (1.0f - r0) * pow(1.0f - cos_i, 5.0f);
}

#if 0
template<typename T, unsigned int N>
inline T any(const vector<T, N> & v) {
	for (unsigned int i = 0; i < N; i++)
		if (v[i])
			return true;

	return false;
}

template<typename T, unsigned int N>
inline T all(const vector<T, N> & v) {
	for (unsigned int i = 0; i < N; i++)
		if (!v[i])
			return false;

	return true;
}

template<typename T, typename B, unsigned int N>
inline vector<T, N> blend(const vector<B, N> & mask, const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = mask[i] ? rhs[i] : lhs[i];

	return out;
}
#endif

#if 0
template<typename T, unsigned int N>
inline vector<bmask, N> operator<(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] < rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator<=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] <= rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator==(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] == rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator>=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] >= rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator>(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] > rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator!=(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<bmask, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] != rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator&&(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] && rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator||(const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = lhs[i] || rhs[i];

	return out;
}

template<typename T, unsigned int N>
inline vector<bmask, N> operator!(const vector<T, N> & v) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = !v[i];

	return out;
}
#endif

template<>
struct ALIGN(16) vector<float, 4> {
	union {
		float _v[4];
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		__m128 _s;
	};

	vector()
		: _s(_mm_setzero_ps())
	{
	}

	vector(float v)
		: _s(_mm_broadcast_ss(&v))
	{
	}

	vector(const vector<float, 3> & xyz, float w)
		: _s(_mm_setr_ps(xyz.x, xyz.y, xyz.z, w)) // TODO
	{
	}

	vector(float x, float y, float z, float w)
		: _s(_mm_setr_ps(x, y, z, w))
	{
	}

	vector(__m128 s)
		: _s(s)
	{
	}

	vector<float, 3> xyz() {
		return vector<float, 3 >(_s);
	}

	float & operator[](unsigned int i) {
		return _v[i];
	}

	const float & operator[](unsigned int i) const {
		return _v[i];
	}
};

inline vector<float, 4> min(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return _mm_min_ps(lhs._s, rhs._s);
}

inline vector<float, 4> max(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return _mm_max_ps(lhs._s, rhs._s);
}

// TODO: abs
// TODO: saturate. Note: SSE has special functions for saturation

inline vector<float, 4> operator+(const vector<float, 4> lhs, const vector<float, 4> & rhs) {
	return _mm_add_ps(lhs._s, rhs._s);
}

inline vector<float, 4> operator-(const vector<float, 4> lhs, const vector<float, 4> & rhs) {
	return _mm_sub_ps(lhs._s, rhs._s);
}

inline vector<float, 4> operator-(const vector<float, 4> & vec) {
	return _mm_sub_ps(_mm_set1_ps(0.0f), vec._s);
}

inline vector<float, 4> operator*(const vector<float, 4> lhs, const vector<float, 4> & rhs) {
	return _mm_mul_ps(lhs._s, rhs._s);
}

inline vector<float, 4> operator/(const vector<float, 4> lhs, const vector<float, 4> & rhs) {
	return _mm_div_ps(lhs._s, rhs._s);
}

inline float dot(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return vector<float, 4>(_mm_dp_ps(lhs._s, rhs._s, 0xFF)).x;
}

inline float length(const vector<float, 4> & vec) {
	return vector<float, 4>(_mm_sqrt_ps(_mm_dp_ps(vec._s, vec._s, 0xFF))).x;
}

inline vector<float, 4> normalize(const vector<float, 4> & vec) {
	return _mm_div_ps(vec._s, _mm_sqrt_ps(_mm_dp_ps(vec._s, vec._s, 0xFF)));
}

#if 0
template<typename T, unsigned int N>
inline T any(const vector<T, N> & v) {
	for (unsigned int i = 0; i < N; i++)
		if (v[i])
			return true;

	return false;
}

template<typename T, unsigned int N>
inline T all(const vector<T, N> & v) {
	for (unsigned int i = 0; i < N; i++)
		if (!v[i])
			return false;

	return true;
}

template<typename T, typename B, unsigned int N>
inline vector<T, N> blend(const vector<B, N> & mask, const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = mask[i] ? rhs[i] : lhs[i];

	return out;
}
#endif

template<>
struct ALIGN(16) vector<bmask, 2> {
	union {
		bmask _v[4];
		struct {
			bmask x;
			bmask y;
			bmask _pad0;
			bmask _pad1;
		};
		__m128 _s;
	};

	vector()
		: _s(_mm_setzero_ps())
	{
	}

	vector(bmask v)
		: _s(_mm_castsi128_ps(_mm_set1_epi32(v)))
	{
	}

#if 0
	vector(const vector<bmask, 3> & xyz, bmask w)
		: _s(_mm_setr_ps(xyz.x, xyz.y, xyz.z, w)) // TODO
	{
	}
#endif

	vector(bmask x, bmask y)
		: _s(_mm_castsi128_ps(_mm_setr_epi32(x, y, 0, 0)))
	{
	}

	vector(__m128 s)
		: _s(s)
	{
	}

#if 0
	vector<bmask, 3> xyz() {
		return vector<bmask, 3 >(_s);
	}
#endif

	bmask & operator[](unsigned int i) {
		return _v[i];
	}

	const bmask & operator[](unsigned int i) const {
		return _v[i];
	}
};

template<>
struct ALIGN(16) vector<bmask, 3> {
	union {
		bmask _v[4];
		struct {
			bmask x;
			bmask y;
			bmask z;
			bmask _pad;
		};
		__m128 _s;
	};

	vector()
		: _s(_mm_setzero_ps())
	{
	}

	vector(bmask v)
		: _s(_mm_castsi128_ps(_mm_set1_epi32(v)))
	{
	}

#if 0
	vector(const vector<bmask, 3> & xyz, bmask w)
		: _s(_mm_setr_ps(xyz.x, xyz.y, xyz.z, w)) // TODO
	{
	}
#endif

	vector(bmask x, bmask y, bmask z)
		: _s(_mm_castsi128_ps(_mm_setr_epi32(x, y, z, 0)))
	{
	}

	vector(__m128 s)
		: _s(s)
	{
	}

#if 0
	vector<bmask, 3> xyz() {
		return vector<bmask, 3 >(_s);
	}
#endif

	bmask & operator[](unsigned int i) {
		return _v[i];
	}

	const bmask & operator[](unsigned int i) const {
		return _v[i];
	}
};

template<>
struct ALIGN(16) vector<bmask, 4> {
	union {
		bmask _v[4];
		struct {
			bmask x;
			bmask y;
			bmask z;
			bmask w;
		};
		__m128 _s;
	};

	vector()
		: _s(_mm_setzero_ps())
	{
	}

	vector(bmask v)
		: _s(_mm_castsi128_ps(_mm_set1_epi32(v)))
	{
	}

#if 0
	vector(const vector<bmask, 3> & xyz, bmask w)
		: _s(_mm_setr_ps(xyz.x, xyz.y, xyz.z, w)) // TODO
	{
	}
#endif

	vector(bmask x, bmask y, bmask z, bmask w)
		: _s(_mm_castsi128_ps(_mm_setr_epi32(x, y, z, w)))
	{
	}

	vector(__m128 s)
		: _s(s)
	{
	}

#if 0
	vector<bmask, 3> xyz() {
		return vector<bmask, 3 >(_s);
	}
#endif

	bmask & operator[](unsigned int i) {
		return _v[i];
	}

	const bmask & operator[](unsigned int i) const {
		return _v[i];
	}
};

inline vector<bmask, 2> operator<(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_cmplt_ps(lhs._s, rhs._s);
}

inline vector<bmask, 2> operator<=(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_cmple_ps(lhs._s, rhs._s);
}

inline vector<bmask, 2> operator==(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_cmpeq_ps(lhs._s, rhs._s);
}

inline vector<bmask, 2> operator>=(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_cmpge_ps(lhs._s, rhs._s);
}

inline vector<bmask, 2> operator>(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_cmpgt_ps(lhs._s, rhs._s);
}

inline vector<bmask, 2> operator!=(const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_cmpneq_ps(lhs._s, rhs._s);
}

inline vector<bmask, 3> operator<(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
    return _mm_cmplt_ps(lhs._s, rhs._s);
}

inline vector<bmask, 3> operator<=(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
    return _mm_cmple_ps(lhs._s, rhs._s);
}

inline vector<bmask, 3> operator==(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
    return _mm_cmpeq_ps(lhs._s, rhs._s);
}

inline vector<bmask, 3> operator>=(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
    return _mm_cmpge_ps(lhs._s, rhs._s);
}

inline vector<bmask, 3> operator>(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
    return _mm_cmpgt_ps(lhs._s, rhs._s);
}

inline vector<bmask, 3> operator!=(const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
    return _mm_cmpneq_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator<(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
    return _mm_cmplt_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator<=(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return _mm_cmple_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator==(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return _mm_cmpeq_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator>=(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return _mm_cmpge_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator>(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return _mm_cmpgt_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator!=(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return _mm_cmpneq_ps(lhs._s, rhs._s);
}

#if 0
template<>
inline vector<bmask, 4> operator&&(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	vector<float, 4> out;

	for (unsigned int i = 0; i < 4; i++)
		out[i] = lhs[i] && rhs[i];

	return out;
}

template<>
inline vector<bmask, 4> operator||(const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	vector<float, 4> out;

	for (unsigned int i = 0; i < 4; i++)
		out[i] = lhs[i] || rhs[i];

	return out;
}

template<>
inline vector<bmask, 4> operator!(const vector<float, 4> & v) {
	vector<float, 4> out;

	for (unsigned int i = 0; i < 4; i++)
		out[i] = !v[i];

	return out;
}
#endif

#if 0
template<typename T, unsigned int N>
inline T any(const vector<T, N> & v) {
	for (unsigned int i = 0; i < N; i++)
		if (v[i])
			return true;

	return false;
}

template<typename T, unsigned int N>
inline T all(const vector<T, N> & v) {
	for (unsigned int i = 0; i < N; i++)
		if (!v[i])
			return false;

	return true;
}

template<typename T, typename B, unsigned int N>
inline vector<T, N> blend(const vector<B, N> & mask, const vector<T, N> & lhs, const vector<T, N> & rhs) {
	vector<T, N> out;

	for (unsigned int i = 0; i < N; i++)
		out[i] = mask[i] ? rhs[i] : lhs[i];

	return out;
}
#endif

inline vector<bmask, 4> operator==(const vector<bmask, 4> & lhs, const vector<bmask, 4> & rhs) {
	return _mm_cmpeq_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator!=(const vector<bmask, 4> & lhs, const vector<bmask, 4> & rhs) {
	return _mm_cmpneq_ps(lhs._s, rhs._s);
}

inline vector<bmask, 2> operator&(const vector<bmask, 2> & lhs, const vector<bmask, 2> & rhs) {
	return _mm_and_ps(lhs._s, rhs._s);
}

inline vector<bmask, 2> operator|(const vector<bmask, 2> & lhs, const vector<bmask, 2> & rhs) {
	return _mm_or_ps(lhs._s, rhs._s);
}

inline vector<bmask, 3> operator&(const vector<bmask, 3> & lhs, const vector<bmask, 3> & rhs) {
	return _mm_and_ps(lhs._s, rhs._s);
}

inline vector<bmask, 3> operator|(const vector<bmask, 3> & lhs, const vector<bmask, 3> & rhs) {
	return _mm_or_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator&(const vector<bmask, 4> & lhs, const vector<bmask, 4> & rhs) {
	return _mm_and_ps(lhs._s, rhs._s);
}

inline vector<bmask, 4> operator|(const vector<bmask, 4> & lhs, const vector<bmask, 4> & rhs) {
	return _mm_or_ps(lhs._s, rhs._s);
}

inline vector<bmask, 2> operator~(const vector<bmask, 2> & v) {
	__m128 mask = _mm_cmpge_ps(_mm_setzero_ps(), _mm_setzero_ps());
	return _mm_andnot_ps(v._s, mask);
}

inline vector<bmask, 3> operator~(const vector<bmask, 3> & v) {
	__m128 mask = _mm_cmpge_ps(_mm_setzero_ps(), _mm_setzero_ps());
	return _mm_andnot_ps(v._s, mask);
}

inline vector<bmask, 4> operator~(const vector<bmask, 4> & v) {
	__m128 mask = _mm_cmpge_ps(_mm_setzero_ps(), _mm_setzero_ps());
	return _mm_andnot_ps(v._s, mask);
}

inline vector<float, 2> blend(const vector<bmask, 2> & mask, const vector<float, 2> & lhs, const vector<float, 2> & rhs) {
	return _mm_blendv_ps(lhs._s, rhs._s, mask._s);
}

inline vector<float, 3> blend(const vector<bmask, 3> & mask, const vector<float, 3> & lhs, const vector<float, 3> & rhs) {
	return _mm_blendv_ps(lhs._s, rhs._s, mask._s);
}

inline vector<float, 4> blend(const vector<bmask, 4> & mask, const vector<float, 4> & lhs, const vector<float, 4> & rhs) {
	return _mm_blendv_ps(lhs._s, rhs._s, mask._s);
}

inline bool none(const vector<bmask, 4> & v) {
	return _mm_movemask_ps(v._s) == 0x00000000;
	// TODO: more efficient way?
}

inline bool any(const vector<bmask, 4> & v) {
	return _mm_movemask_ps(v._s) != 0x00000000;
	// TODO: more efficient way?
}

inline bool all(const vector<bmask, 4> & v) {
	return _mm_movemask_ps(v._s) == 0x0000000F;
	// TODO: more efficient way?
}

template<>
struct ALIGN(16) vector<int, 2> {
	union {
		int _v[4];
		struct {
			int x;
			int y;
			int _pad0;
			int _pad1;
		};
		__m128i _s;
	};

	vector()
		: _s(_mm_setzero_si128())
	{
	}

	vector(int v)
		: _s(_mm_set1_epi32(v))
	{
	}

	vector(int x, int y)
		: _s(_mm_setr_epi32(x, y, 0, 0))
	{
	}

	vector(__m128i s)
		: _s(s)
	{
	}

	int & operator[](unsigned int i) {
		return _v[i];
	}

	const int & operator[](unsigned int i) const {
		return _v[i];
	}
};

template<>
struct ALIGN(16) vector<int, 3> {
	union {
		int _v[4];
		struct {
			int x;
			int y;
			int z;
			int _pad0;
		};
		__m128i _s;
	};

	vector()
		: _s(_mm_setzero_si128())
	{
	}

	vector(int v)
		: _s(_mm_set1_epi32(v))
	{
	}

	vector(int x, int y, int z)
		: _s(_mm_setr_epi32(x, y, z, 0))
	{
	}

	vector(__m128i s)
		: _s(s)
	{
	}

	int & operator[](unsigned int i) {
		return _v[i];
	}

	const int & operator[](unsigned int i) const {
		return _v[i];
	}
};

template<>
struct ALIGN(16) vector<int, 4> {
	union {
		int _v[4];
		struct {
			int x;
			int y;
			int z;
			int w;
		};
		__m128i _s;
	};

	vector()
		: _s(_mm_setzero_si128())
	{
	}

	vector(int v)
		: _s(_mm_set1_epi32(v))
	{
	}

	vector(int x, int y, int z, int w)
		: _s(_mm_setr_epi32(x, y, z, w))
	{
	}

	vector(__m128i s)
		: _s(s)
	{
	}

	int & operator[](unsigned int i) {
		return _v[i];
	}

	const int & operator[](unsigned int i) const {
		return _v[i];
	}
};

inline vector<int, 4> blend(const vector<bmask, 4> & mask, const vector<int, 4> & lhs, const vector<int, 4> & rhs) {
	// TODO: Conversion between float and int vector pipes may be a problem
	// TODO: make sure the masking is compatible
	return _mm_blendv_epi8(lhs._s, rhs._s, _mm_castps_si128(mask._s));
}

#endif
#endif
