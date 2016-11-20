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
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

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
struct vector {
    T _v[N];
    
    vector<T, N>()
        : _v()
    {
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

typedef vector<unsigned int, 2> uint2;
typedef vector<unsigned int, 3> uint3;
typedef vector<unsigned int, 4> uint4;

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

#endif
