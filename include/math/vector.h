/**
 * @file vector.h
 *
 * @brief Vector type
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_VECTOR_H
#define __MATH_VECTOR_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <xmmintrin.h>
#include <smmintrin.h>

// TODO: Some of the functions IN vector classes might be optimized with SSE
// instructions.
// TODO: SSE for other types and sizes

union simd_vec4 {
    float  f32[4];
    __m128 m128;
};

template<typename T, unsigned int N, bool SIMD>
struct vector_data {
    T v[N];
};

template<unsigned int N>
struct vector_data<float, N, true> {
    union {
        float v[N];
        __m128 simd[(N + 3) / 4];
    };
};

template<typename T, unsigned int N, bool SIMD = false>
struct vector : public vector_data<T, N, SIMD> {
    vector<T, N>() {
        for (unsigned int i = 0; i < N; i++)
            vector_data<T, N, SIMD>::v[i] = 0;
    }

    vector<T, N>(T fill) {
        for (unsigned int i = 0; i < N; i++)
            vector_data<T, N, SIMD>::v[i] = fill;
    }
};

#define MATH_DEFAULT_SIMD

#ifndef MATH_DEFAULT_SIMD
typedef vector<float, 2, false> vec2;
typedef vector<float, 3, false> vec3;
typedef vector<float, 4, false> vec4;
#else
typedef vector<float, 2, true> vec2;
typedef vector<float, 3, true> vec3;
typedef vector<float, 4, true> vec4;
#endif

// TODO initialization list

template<typename T>
struct vector<T, 2, false> {
    union {
        T v[2];
        struct {
            T x;
            T y;
        };
    };

    vector<T, 2, false>()
        : x(0),
          y(0)
    {
    }

    vector<T, 2, false>(T fill)
        : x(fill),
          y(fill)
    {
    }

    vector<T, 2, false>(T x, T y)
        : x(x),
          y(y)
    {
    }
};

template <>
struct vector<float, 2, true> {
    union {
        float v[2];
        struct {
            float x;
            float y;
        };
        __m128 simd[1];
    };

    vector<float, 2, true>()
        : simd { _mm_setzero_ps() }
    {
    }

    vector<float, 2, true>(__m128 simd)
        : simd { simd }
    {
    }

    vector<float, 2, true>(float fill)
        : simd { _mm_set1_ps(fill) }
    {
    }

    vector<float, 2, true>(float x, float y)
        : simd { _mm_setr_ps(x, y, 0.0f, 0.0f) }
    {
    }
};

template<typename T>
struct vector<T, 3, false> {
    union {
        T v[3];
        struct {
            T x;
            T y;
            T z;
        };
    };

    vector<T, 3, false>()
        : x(0),
          y(0),
          z(0)
    {
    }

    vector<T, 3, false>(T fill)
        : x(fill),
          y(fill),
          z(fill)
    {
    }

    vector<T, 3, false>(T x, T y, T z)
        : x(x),
          y(y),
          z(z)
    {
    }
};

template<>
struct vector<float, 3, true> {
    union {
        float v[3];
        struct {
            float x;
            float y;
            float z;
        };
        __m128 simd[1];
    };

    vector<float, 3, true>()
        : simd { _mm_setzero_ps() }
    {
    }

    vector<float, 3, true>(__m128 simd)
        : simd { simd }
    {
    }

    vector<float, 3, true>(float fill)
        : simd { _mm_set1_ps(fill) }
    {
    }

    vector<float, 3, true>(float x, float y, float z)
        : simd { _mm_setr_ps(x, y, z, 0.0f) }
    {
    }
};

template<typename T>
struct vector<T, 4, false> {
    union {
        T v[4];
        struct {
            T x;
            T y;
            T z;
            T w;
        };
    };

    vector<T, 4, false>()
        : x(0),
          y(0),
          z(0),
          w(0)
    {
    }

    vector<T, 4, false>(T fill)
        : x(fill),
          y(fill),
          z(fill),
          w(fill)
    {
    }

    vector<T, 4, false>(T x, T y, T z, T w)
        : x(x),
          y(y),
          z(z),
          w(w)
    {
    }

    vector<T, 4, false>(vector<T, 3, false> v3, T w)
        : x(v3.x),
          y(v3.y),
          z(v3.z),
          w(w)
    {
    }

    vector<T, 3, false> xyz() {
        return vector<T, 3, false>(x, y, z);
    }
};

template<>
struct vector<float, 4, true> {
    union {
        float v[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        __m128 simd[1];
    };

    vector<float, 4, true>()
        : simd { _mm_setzero_ps() }
    {
    }

    vector<float, 4, true>(__m128 simd)
        : simd { simd }
    {
    }

    vector<float, 4, true>(float fill)
        : simd { _mm_set1_ps(fill) }
    {
    }

    vector<float, 4, true>(float x, float y, float z, float w)
        : simd { _mm_setr_ps(x, y, z, w) }
    {
    }

    vector<float, 4, true>(vector<float, 3, true> v3, float w)
        : simd { _mm_setr_ps(v3.x, v3.y, v3.z, w) } // TODO might be a better way
    {
    }

    vector<float, 3, true> xyz() {
        return vector<float, 3, true>(simd[0]);
    }
};

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> & operator+=(vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] += rhs.v[i];
    return lhs;
}

template<unsigned int N>
inline vector<float, N, true> & operator+=(vector<float, N, true> & lhs, const vector<float, N, true> & rhs) {
    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        lhs.simd[i] = _mm_add_ps(lhs.simd[i], rhs.simd[i]);
    return lhs;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator+(const vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val = lhs;
    val += rhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> & operator-=(vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] -= rhs.v[i];
    return lhs;
}

template<unsigned int N>
inline vector<float, N, true> & operator-=(vector<float, N, true> & lhs, const vector<float, N, true> & rhs) {
    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        lhs.simd[i] = _mm_sub_ps(lhs.simd[i], rhs.simd[i]);
    return lhs;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator-(const vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val = lhs;
    val -= rhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> & operator*=(vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] *= rhs.v[i];
    return lhs;
}

template<unsigned int N>
inline vector<float, N, true> & operator*=(vector<float, N, true> & lhs, const vector<float, N, true> & rhs) {
    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        lhs.simd[i] = _mm_mul_ps(lhs.simd[i], rhs.simd[i]);
    return lhs;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator*(const vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val = lhs;
    val *= rhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> & operator/=(vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] /= rhs.v[i];
    return lhs;
}

template<unsigned int N>
inline vector<float, N, true> & operator/=(vector<float, N, true> & lhs, const vector<float, N, true> & rhs) {
    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        lhs.simd[i] = _mm_div_ps(lhs.simd[i], rhs.simd[i]);
    return lhs;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator/(const vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val = lhs;
    val /= rhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> & operator+=(vector<T, N, SIMD> & lhs, T rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] += rhs;
    return lhs;
}

// TODO: Maybe there's a way to load constants outside the functions in case
// they're reused?

template<unsigned int N>
inline vector<float, N, true> & operator+=(vector<float, N, true> & lhs, float rhs) {
    __m128 rhs_simd = _mm_set1_ps(rhs);

    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        lhs.simd[i] = _mm_add_ps(lhs.simd[i], rhs_simd);

    return lhs;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator+(const vector<T, N, SIMD> & lhs, T rhs) {
    vector<T, N, SIMD> val = lhs;
    val += rhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator+(T lhs, const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val = rhs;
    val += lhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> & operator-=(vector<T, N, SIMD> & lhs, T rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] -= rhs;
    return lhs;
}

template<unsigned int N>
inline vector<float, N, true> & operator-=(vector<float, N, true> & lhs, float rhs) {
    __m128 rhs_simd = _mm_set1_ps(rhs);

    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        lhs.simd[i] = _mm_sub_ps(lhs.simd[i], rhs_simd);

    return lhs;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator-(const vector<T, N, SIMD> & lhs, T rhs) {
    vector<T, N, SIMD> val = lhs;
    val -= rhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator-(T lhs, const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val = rhs;
    val -= lhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> & operator*=(vector<T, N, SIMD> & lhs, T rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] *= rhs;
    return lhs;
}

template<unsigned int N>
inline vector<float, N, true> & operator*=(vector<float, N, true> & lhs, float rhs) {
    __m128 rhs_simd = _mm_set1_ps(rhs);

    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        lhs.simd[i] = _mm_mul_ps(lhs.simd[i], rhs_simd);

    return lhs;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator*(const vector<T, N, SIMD> & lhs, T rhs) {
    vector<T, N, SIMD> val = lhs;
    val *= rhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator*(T lhs, const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val = rhs;
    val *= lhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> & operator/=(vector<T, N, SIMD> & lhs, T rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] /= rhs;
    return lhs;
}

template<unsigned int N>
inline vector<float, N, true> & operator/=(vector<float, N, true> & lhs, float rhs) {
    __m128 rhs_simd = _mm_set1_ps(rhs);

    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        lhs.simd[i] = _mm_div_ps(lhs.simd[i], rhs_simd);

    return lhs;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator/(const vector<T, N, SIMD> & lhs, T rhs) {
    vector<T, N, SIMD> val = lhs;
    val /= rhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator/(T lhs, const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val = rhs;
    val /= lhs;
    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> operator-(const vector<T, N, SIMD> & rhs) {
    vector<T, N, SIMD> val;

    for (unsigned int i = 0; i < N; i++)
        val.v[i] = -rhs.v[i];

    return val;
}

template<unsigned int N>
inline vector<float, N, true> operator-(vector<float, N, true> & rhs) {
    vector<float, N, true> val;
    __m128 zero = _mm_setzero_ps();

    for (unsigned int i = 0; i < (N + 3) / 4; i++)
        val.simd[i] = _mm_sub_ps(zero, rhs.simd[i]);

    return val;
}

template<typename T, unsigned int N, bool SIMD>
inline T dot(const vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    T result = 0;

    for (unsigned int i = 0; i < N; i++)
        result += lhs.v[i] * rhs.v[i];

    return result;
}

template<unsigned int N>
inline float dot(const vector<float, N, true> & lhs, const vector<float, N, true> & rhs) {
    float result = 0.0f;

    // Note: The optimizer should unroll these loops and compute the mask statically

    for (unsigned int i = 0; i < (N + 3) / 4; i++) {
        int mask = 1;

        unsigned int maskIdx = 4;
        for (unsigned int j = i; j < N; j++)
            mask |= (1 << maskIdx++);

        vec4 dp(_mm_dp_ps(lhs.simd[i], rhs.simd[i], mask));
        result += dp.x;
    }

    return result;
}

template<typename T, unsigned int N, bool SIMD>
inline T length2(const vector<T, N, SIMD> & vec) {
    return dot(vec, vec);
}

template<typename T, unsigned int N, bool SIMD>
inline T length(const vector<T, N, SIMD> & vec) {
    return static_cast<T>(sqrt(dot(vec, vec)));
}

template<unsigned int N, bool SIMD>
inline float length(const vector<float, N, SIMD> & vec) {
    return sqrtf(dot(vec, vec));
}

template<typename T, unsigned int N, bool SIMD>
inline vector<T, N, SIMD> normalize(const vector<T, N, SIMD> & vec) {
    T len = length(vec);
    return vec / len;
}

template<typename T, bool SIMD>
inline vector<T, 3, SIMD> cross(const vector<T, 3, SIMD> & lhs, const vector<T, 3, SIMD> & rhs) {
    return vector<T, 3, SIMD>(
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x);
}

/**
 * Reflect this vector across a normal vector. TODO.
 */
template<typename T, bool SIMD>
inline vector<T, 3, SIMD> reflect(const vector<T, 3, SIMD> & vec, const vector<T, 3, SIMD> & across) {
    return vec + across * (-2 * dot(vec, across));
}

/**
 * Get a refracted vector according to Snell's law. Assumes that this vector points towards
 * the interface. TODO.
 *
 * @param norm Normal at interface
 * @param n1   Index of refraction of material being left
 * @param n2   Index of refractio nof material being entered
 */
template<typename T, bool SIMD>
inline vector<T, 3, SIMD> refract(const vector<T, 3, SIMD> & vec, const vector<T, 3, SIMD> & norm, float n1, float n2) {
    vector<T, 3, SIMD> L = -vec;
    vector<T, 3, SIMD> N = norm;

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
template<typename T, bool SIMD>
inline float schlick(const vector<T, 3, SIMD> & n, const vector<T, 3, SIMD> & v, float n1, float n2) {
    // TODO specular highlights can use this too?

    float cos_i = dot(n, v);

    float r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;

    // TODO: total internal reflection

    return r0 + (1.0f - r0) * pow(1.0f - cos_i, 5.0f);
}

#endif
