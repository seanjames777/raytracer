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
#include <type_traits>

// TODO: Some of the functions IN vector classes might be optimized with SSE
// instructions.
// TODO: SSE for other types and sizes

template<typename T, unsigned int N, bool SIMD>
struct vector_data {
    T v[N];
};

template<typename T, unsigned int N>
struct vector_data<T, N, true> {
    union {
        T v[N];
        __m128 simd[(N + 3) / 4];
    };
};

template<typename T, unsigned int N, bool SIMD>
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

typedef vector<float, 2, false> vec2;
typedef vector<float, 3, false> vec3;
typedef vector<float, 4, false> vec4;

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
        : x(0),
          y(0)
    {
    }

    vector<float, 2, true>(float fill)
        : x(fill),
          y(fill)
    {
    }

    vector<float, 2, true>(float x, float y)
        : x(x),
          y(y)
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
        : x(0),
          y(0),
          z(0)
    {
    }

    vector<float, 3, true>(float fill)
        : x(fill),
          y(fill),
          z(fill)
    {
    }

    vector<float, 3, true>(float x, float y, float z)
        : x(x),
          y(y),
          z(z)
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
        : x(0),
          y(0),
          z(0),
          w(0)
    {
    }

    vector<float, 4, true>(float fill)
        : x(fill),
          y(fill),
          z(fill),
          w(fill)
    {
    }

    vector<float, 4, true>(float x, float y, float z, float w)
        : x(x),
          y(y),
          z(z),
          w(w)
    {
    }

    vector<float, 4, true>(vector<float, 3, true> v3, float w)
        : x(v3.x),
          y(v3.y),
          z(v3.z),
          w(w)
    {
    }

    vector<float, 3, true> xyz() {
        return vector<float, 3, true>(x, y, z);
    }
};

template<typename T, unsigned int N>
inline vector<T, N, false> & operator+=(vector<T, N, false> & lhs, const vector<T, N, false> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] += rhs.v[i];
    return lhs;
}

template<unsigned int N>
inline vector<float, N, true> & operator+=(vector<float, N, true> & lhs, const vector<float, N, true> & rhs) {
    for (unsigned int i = 0; i < N / 4; i += 4)
        lhs.v[i] += rhs.v[i];
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

template<typename T, unsigned int N, bool SIMD>
inline T dot(const vector<T, N, SIMD> & lhs, const vector<T, N, SIMD> & rhs) {
    T result = 0;

    for (unsigned int i = 0; i < N; i++)
        result += lhs.v[i] * rhs.v[i];

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
