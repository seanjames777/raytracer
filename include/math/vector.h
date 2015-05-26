/**
 * @file math/vector.h
 *
 * @brief Vector type
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_VECTOR_H
#define __MATH_VECTOR_H

#define _USE_MATH_DEFINES
#include <math.h>

template<typename T, unsigned int N>
struct vector_data {
    T v[N];
};

template<typename T, unsigned int N>
struct vector : public vector_data<T, N> {
    vector<T, N>() {
        for (unsigned int i = 0; i < N; i++)
            vector_data<T, N>::v[i] = 0;
    }

    vector<T, N>(T fill) {
        for (unsigned int i = 0; i < N; i++)
            vector_data<T, N>::v[i] = fill;
    }
};

//#define MATH_DEFAULT_SIMD

typedef vector<float, 2> vec2;
typedef vector<float, 3> vec3;
typedef vector<float, 4> vec4;

// TODO initialization list

template<typename T>
struct vector<T, 2> {
    union {
        T v[2];
        struct {
            T x;
            T y;
        };
    };

    vector<T, 2>()
        : x(0),
          y(0)
    {
    }

    vector<T, 2>(T fill)
        : x(fill),
          y(fill)
    {
    }

    vector<T, 2>(T x, T y)
        : x(x),
          y(y)
    {
    }
};

template<typename T>
struct vector<T, 3> {
    union {
        T v[3];
        struct {
            T x;
            T y;
            T z;
        };
    };

    vector<T, 3>()
        : x(0),
          y(0),
          z(0)
    {
    }

    vector<T, 3>(T fill)
        : x(fill),
          y(fill),
          z(fill)
    {
    }

    vector<T, 3>(T x, T y, T z)
        : x(x),
          y(y),
          z(z)
    {
    }

    vector<T, 3>(const vector<T, 2> & xy, T z)
        : x(xy.x),
          y(xy.y),
          z(z)
    {
    }
};

template<typename T>
struct vector<T, 4> {
    union {
        T v[4];
        struct {
            T x;
            T y;
            T z;
            T w;
        };
    };

    vector<T, 4>()
        : x(0),
          y(0),
          z(0),
          w(0)
    {
    }

    vector<T, 4>(T fill)
        : x(fill),
          y(fill),
          z(fill),
          w(fill)
    {
    }

    vector<T, 4>(T x, T y, T z, T w)
        : x(x),
          y(y),
          z(z),
          w(w)
    {
    }

    vector<T, 4>(vector<T, 3> v3, T w)
        : x(v3.x),
          y(v3.y),
          z(v3.z),
          w(w)
    {
    }

    vector<T, 4>(vector<T, 3> v3)
        : x(v3.x),
          y(v3.y),
          z(v3.z),
          w(1.0f)
    {
    }

    vector<T, 3> xyz() const {
        return vector<T, 3>(x, y, z);
    }
};

template<typename T, unsigned int N>
inline vector<T, N> & operator+=(vector<T, N> & lhs, const vector<T, N> & rhs) {
    for (unsigned int i = 0; i < N; i++)
        lhs.v[i] += rhs.v[i];
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
        lhs.v[i] -= rhs.v[i];
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
        lhs.v[i] *= rhs.v[i];
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
        lhs.v[i] /= rhs.v[i];
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
        lhs.v[i] += rhs;
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
        lhs.v[i] -= rhs;
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
        lhs.v[i] *= rhs;
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
        lhs.v[i] /= rhs;
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
        val.v[i] = -rhs.v[i];

    return val;
}

template<typename T, unsigned int N>
inline T dot(const vector<T, N> & lhs, const vector<T, N> & rhs) {
    T result = 0;

    for (unsigned int i = 0; i < N; i++)
        result += lhs.v[i] * rhs.v[i];

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
inline vector<T, N> normalize(const vector<T, N> & vec) {
    T len = length(vec);
    return vec / len;
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
