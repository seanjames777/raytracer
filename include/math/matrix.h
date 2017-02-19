/**
 * @file math/matrix.h
 *
 * @brief Matrix type
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_MATRIX_H
#define __MATH_MATRIX_H

#include <math/vector.h>
#include <math/ray.h>

#define NDC_ZERO_TO_ONE 0

// M rows x N columns. TODO.
template<typename T, unsigned int M, unsigned int N>
struct matrix {
    
    vector<T, N> rows[M];

    // TODO initializer list, and zero()

    matrix() {
        for (unsigned int j = 0; j < M; j++)
            for (unsigned int i = 0; i < N; i++)
                rows[j][i] = j == i ? (T)1 : (T)0;
    }
};

typedef matrix<float, 3, 3> float3x3;
typedef matrix<float, 4, 4> float4x4;

template<typename T, unsigned int M, unsigned int N>
matrix<T, M, N> zero() {
    matrix<T, M, N> mat;

    for (unsigned int j = 0; j < M; j++)
        for (unsigned int i = 0; i < N; i++)
            mat.rows[j][i] = 0;

    return mat;
}

template<typename T, unsigned int N>
vector<T, N> operator*(const vector<T, N> & v, const matrix<T, N, N> & m) {
    vector<T, N> result;

    for (unsigned int i = 0; i < N; i++)
        for (unsigned int j = 0; j < N; j++)
            result[i] += v[i] * m.rows[j][i];

    return result;
}

template<typename T, unsigned int N>
vector<T, N> operator*(const matrix<T, N, N> & m, const vector<T, N> & v) {
    vector<T, N> result;

    for (unsigned int i = 0; i < N; i++)
        for (unsigned int j = 0; j < N; j++)
            result[i] += m.rows[i][j] * v[j];

    return result;
}

template<typename T, unsigned int M, unsigned int N, unsigned int P>
matrix<T, M, P> operator*(const matrix<T, M, N> & lhs, const matrix<T, N, P> & rhs) {
    matrix<T, M, P> result = zero<T, M, P>();

    for (unsigned int j = 0; j < M; j++)
        for (unsigned int i = 0; i < P; i++)
            for (unsigned int k = 0; k < N; k++)
                result.rows[j][i] += lhs.rows[j][k] * rhs.rows[k][i];

    return result;
}

template<typename T, unsigned int N>
matrix<T, N, N> transpose(const matrix<T, N, N> & mat) {
    matrix<T, N, N> result; // TODO don't need identityness

    for (unsigned int j = 0; j < N; j++)
        for (unsigned int i = 0; i < N; i++)
            result.rows[j][i] = mat.rows[i][j];

    return result;
}

/**
 * Get a left handed orthographic projection matrix. TODO linline/static
 */
static float4x4 orthographicLH(float left, float right, float bottom, float top, float nearz, float farz) {
    float4x4 out;

    out.rows[0][0]  = 2.0f / (right - left);
    out.rows[1][1]  = 2.0f / (top - bottom);
    out.rows[2][2] = 1.0f / (farz - nearz);
    out.rows[3][0] = (left + right) / (left - right);
    out.rows[3][1] = (top + bottom) / (bottom - top);
    out.rows[3][2] = nearz / (nearz - farz);
    out.rows[3][3] = 1.0f;

    return out;
}

/**
 * Get a left handed orthographic projection matrix centered around the origin
 */
static float4x4 orthographicLH(float width, float height, float depth) {
    return orthographicLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, 0.01f, depth);
}

/**
 * Get a right handed orthographic projection matrix
 */
static float4x4 orthographicRH(float left, float right, float bottom, float top, float nearz, float farz) {
    float4x4 out;

    out.rows[0][0]  = 2.0f / (right - left);
    out.rows[1][1]  = 2.0f / (top - bottom);
    out.rows[2][2] = 1.0f / (farz - nearz);
    out.rows[3][0] = (left + right) / (left - right);
    out.rows[3][1] = (top + bottom) / (bottom - top);
    out.rows[3][2] = nearz / (nearz - farz);
    out.rows[3][3] = 1.0f;

    return out;
}

/**
 * Get a right handed orthographic projection matrix centered around the origin
 */
static float4x4 orthographicRH(float width, float height, float depth) {
    return orthographicRH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, -depth / 2.0f, depth / 2.0f);
}

/**
 * Create a left handed look-at matrix
 */
static float4x4 lookAtLH(const float3 & position, const float3 & target, const float3 & up) {
    float3 zAxis = normalize(target - position);
    float3 xAxis = normalize(cross(up, zAxis));
    float3 yAxis = normalize(cross(zAxis, xAxis));

    float4x4 out;

    out.rows[0][0] = xAxis.x;
    out.rows[0][1] = xAxis.y;
    out.rows[0][2] = xAxis.z;
    out.rows[0][3] = dot(-xAxis, position);

    out.rows[1][0] = yAxis.x;
    out.rows[1][1] = yAxis.y;
    out.rows[1][2] = yAxis.z;
    out.rows[1][3] = dot(-yAxis, position);

    out.rows[2][0] = zAxis.x;
    out.rows[2][1] = zAxis.y;
    out.rows[2][2] = zAxis.z;
    out.rows[2][3] = dot(-zAxis, position);

    out.rows[3][0] = 0;
    out.rows[3][1] = 0;
    out.rows[3][2] = 0;
    out.rows[3][3] = 1;

    return out;
}

/**
 * Create a right handed look-at matrix
 */
static float4x4 lookAtRH(const float3 & position, const float3 & target, const float3 & up) {
    float3 zAxis = normalize(target - position);
    float3 xAxis = normalize(cross(up, zAxis));
    float3 yAxis = cross(zAxis, xAxis);

    float4x4 out;

    out.rows[0][0] = xAxis.x;
    out.rows[0][1] = yAxis.x;
    out.rows[0][2] = zAxis.x;
    out.rows[0][3] = 0;

    out.rows[1][0] = xAxis.y;
    out.rows[1][1] = yAxis.y;
    out.rows[1][2] = zAxis.y;
    out.rows[1][3] = 0;

    out.rows[2][0] = xAxis.z;
    out.rows[2][1] = yAxis.z;
    out.rows[2][2] = zAxis.z;
    out.rows[2][3] = 0;

    out.rows[3][0] = dot(xAxis, position);
    out.rows[3][1] = dot(yAxis, position);
    out.rows[3][2] = dot(zAxis, position);
    out.rows[3][3] = 1;

    return out;
}

/**
 * Create a left handed perspective projection matrix
 */
static float4x4 perspectiveLH(float fov, float aspect, float znear, float zfar) {
    float h = cos(fov / 2.0f) / sin(fov / 2.0f);
    float w = h / aspect;

    float4x4 out;

    out.rows[0][0] = w;
    out.rows[1][1] = h;
#if NDC_ZERO_TO_ONE
    out.rows[2][2] = zfar / (zfar - znear);
    out.rows[2][3] = -znear * zfar / (zfar - znear);
#else
    out.rows[2][2] = (zfar + znear) / (zfar - znear);
    out.rows[2][3] = -2 * znear * zfar / (zfar - znear);
#endif
    out.rows[3][2] = 1.0f;

    return out;
}

/**
 * Create a right handed perspective projection matrix
 */
static float4x4 perspectiveRH(float fov, float aspect, float znear, float zfar) {
    float h = cos(fov / 2.0f) / sin(fov / 2.0f);
    float w = h / aspect;

    float4x4 out;

    out.rows[0][0] = w;
    out.rows[1][1] = h;
#if NDC_ZERO_TO_ONE
    out.rows[2][2] = zfar / (zfar - znear);
    out.rows[2][3] = -znear * zfar / (zfar - znear);
#else
    out.rows[2][2] = -(zfar + znear) / (zfar - znear);
    out.rows[2][3] = -2 * znear * zfar / (zfar - znear);
#endif
    out.rows[3][2] = -1.0f;

    return out;
}

/**
 * Get a rotation matrix around the X axis
 */
static float4x4 rotationX(float radians) {
    float cost = cosf(radians);
    float sint = sinf(radians);

    float4x4 out;

    out.rows[0][0] = 1.0f;
    out.rows[1][1] = cost;
    out.rows[1][2] = -sint;
    out.rows[2][1] = sint;
    out.rows[2][2] = cost;
    out.rows[3][3] = 1.0f;

    return out;
}

/**
 * Get a rotation matrix around the Y axis
 */
static float4x4 rotationY(float radians) {
    float cost = cosf(radians);
    float sint = sinf(radians);

    float4x4 out;

    out.rows[0][0] = cost;
    out.rows[0][2] = sint;
    out.rows[1][1] = 1.0f;
    out.rows[2][0] = -sint;
    out.rows[2][2] = cost;
    out.rows[3][3] = 1.0f;

    return out;
}

/**
 * Get a rotation matrix around the Z axis
 */
static float4x4 rotationZ(float radians) {
    float cost = cosf(radians);
    float sint = sinf(radians);

    float4x4 out;

    out.rows[0][0] = cost;
    out.rows[0][1] = -sint;
    out.rows[1][0] = sint;
    out.rows[1][1] = cost;
    out.rows[2][2] = 1.0f;
    out.rows[3][3] = 1.0f;

    return out;
}

/**
 * Get a rotation matrix around an aritrary
 */
static float4x4 rotation(const float3 & axis, float radians) {
    float cost = cosf(radians);
    float sint = sinf(radians);

    float4x4 out;

    out.rows[0][0] = cost + axis.x * axis.x * (1 - cost);
    out.rows[0][1] = (1 - cost) - axis.x * sint;
    out.rows[0][2] = axis.x * axis.z * (1 - cost) + axis.y * sint;
    out.rows[1][0] = axis.y * axis.x * (1 - cost) + axis.x * sint;
    out.rows[1][1] = cost + axis.y * axis.y * (1 - cost);
    out.rows[1][2] = axis.y * axis.z * (1 - cost) - axis.x * sint;
    out.rows[2][0] = axis.z * axis.x * (1 - cost) - axis.y * sint;
    out.rows[2][1] = axis.z * axis.y * (1 - cost) + axis.x * sint;
    out.rows[2][2] = cost + axis.z * axis.z * (1 - cost);
    out.rows[2][3] = 1.0f;

    return out;
}

/**
 * Create a yaw, pitch, roll rotation matrix. The operations are applied
 * in the order: roll, then pitch, then yaw, around the origin
 */
static float4x4 rotation(const float3 & xyz) {
#if 0
    float4x4 out;

    float cy = cosf(xyz.y);
    float sy = sinf(xyz.y);
    float cp = cosf(xyz.x);
    float sp = sinf(xyz.x);
    float cr = cosf(xyz.z);
    float sr = sinf(xyz.z);

    // Generated by mathematica
    out.rows[0][0] = cr * cy + sp * sr * sy;
    out.rows[0][1] = -cy * sr + cr * sp * sy;
    out.rows[0][2] = cp * sy;
    out.rows[0][3] = 0.0f;
    out.rows[1][0] = cp * sr;
    out.rows[1][1] = cp * cr;
    out.rows[1][2] = -sp;
    out.rows[1][3] = 0.0f;
    out.rows[2][0] = cy * sp * sr - cr * sy;
    out.rows[2][1] = cr * cy * sp + sr * sy;
    out.rows[2][2] = cp * cy;
    out.rows[2][3] = 0.0f;
    out.rows[3][0] = 0.0f;
    out.rows[3][1] = 0.0f;
    out.rows[3][2] = 0.0f;
    out.rows[3][3] = 1.0f;

    return out;
#endif

    return rotationZ(xyz.z) * rotationY(xyz.y) * rotationX(xyz.x);
}

/**
 * Get a scaling matrix
 */
static float4x4 scale(const float3 & scale) {
    float4x4 out;

    out.rows[0][0] = scale.x;
    out.rows[1][1] = scale.y;
    out.rows[2][2] = scale.z;

    return out;
}

/**
 * Get a translation matrix
 */
static float4x4 translation(const float3 & translation) {
    float4x4 out;

    out.rows[0][3] = translation.x;
    out.rows[1][3] = translation.y;
    out.rows[2][3] = translation.z;

    return out;
}

/**
 * Get the determinant of a matrix
 */
static float determinant(const float3x3 & mat) {
    // Generated by mathematica
    return -mat.rows[0][2] * mat.rows[1][1] * mat.rows[2][0] +
            mat.rows[0][1] * mat.rows[1][2] * mat.rows[2][0] +
            mat.rows[0][2] * mat.rows[1][0] * mat.rows[2][1] -
            mat.rows[0][0] * mat.rows[1][2] * mat.rows[2][1] -
            mat.rows[0][1] * mat.rows[1][0] * mat.rows[2][2] +
            mat.rows[0][0] * mat.rows[1][1] * mat.rows[2][2];
}

/**
 * Get the determinant of a matrix
 */
static float determinant(const float4x4 & mat) {
    // Generated by mathematica
    return mat.rows[0][3] * mat.rows[1][2] * mat.rows[2][1] * mat.rows[3][0] - mat.rows[0][2] * mat.rows[1][3] * mat.rows[2][1] * mat.rows[3][0] - mat.rows[0][3] * mat.rows[1][1] * mat.rows[2][2] * mat.rows[3][0] +
           mat.rows[0][1] * mat.rows[1][3] * mat.rows[2][2] * mat.rows[3][0] + mat.rows[0][2] * mat.rows[1][1] * mat.rows[2][3] * mat.rows[3][0] - mat.rows[0][1] * mat.rows[1][2] * mat.rows[2][3] * mat.rows[3][0] -
           mat.rows[0][3] * mat.rows[1][2] * mat.rows[2][0] * mat.rows[3][1] + mat.rows[0][2] * mat.rows[1][3] * mat.rows[2][0] * mat.rows[3][1] + mat.rows[0][3] * mat.rows[1][0] * mat.rows[2][2] * mat.rows[3][1] -
           mat.rows[0][0] * mat.rows[1][3] * mat.rows[2][2] * mat.rows[3][1] - mat.rows[0][2] * mat.rows[1][0] * mat.rows[2][3] * mat.rows[3][1] + mat.rows[0][0] * mat.rows[1][2] * mat.rows[2][3] * mat.rows[3][1] +
           mat.rows[0][3] * mat.rows[1][1] * mat.rows[2][0] * mat.rows[3][2] - mat.rows[0][1] * mat.rows[1][3] * mat.rows[2][0] * mat.rows[3][2] - mat.rows[0][3] * mat.rows[1][0] * mat.rows[2][1] * mat.rows[3][2] +
           mat.rows[0][0] * mat.rows[1][3] * mat.rows[2][1] * mat.rows[3][2] + mat.rows[0][1] * mat.rows[1][0] * mat.rows[2][3] * mat.rows[3][2] - mat.rows[0][0] * mat.rows[1][1] * mat.rows[2][3] * mat.rows[3][2] -
           mat.rows[0][2] * mat.rows[1][1] * mat.rows[2][0] * mat.rows[3][3] + mat.rows[0][1] * mat.rows[1][2] * mat.rows[2][0] * mat.rows[3][3] + mat.rows[0][2] * mat.rows[1][0] * mat.rows[2][1] * mat.rows[3][3] -
           mat.rows[0][0] * mat.rows[1][2] * mat.rows[2][1] * mat.rows[3][3] - mat.rows[0][1] * mat.rows[1][0] * mat.rows[2][2] * mat.rows[3][3] + mat.rows[0][0] * mat.rows[1][1] * mat.rows[2][2] * mat.rows[3][3];
}

/**
 * Get the inverse of a matrix
 */
static float3x3 inverse(const float3x3 & mat) {
    float3x3 out;

    float det = determinant(mat);

    // Generated by mathematica
    out.rows[0][0] = -mat.rows[1][2] * mat.rows[2][1] + mat.rows[1][1] * mat.rows[2][2];
    out.rows[0][1] =  mat.rows[0][2] * mat.rows[2][1] - mat.rows[0][1] * mat.rows[2][2];
    out.rows[0][2] = -mat.rows[0][2] * mat.rows[1][1] + mat.rows[0][1] * mat.rows[1][2];
    out.rows[1][0] =  mat.rows[1][2] * mat.rows[2][0] - mat.rows[1][0] * mat.rows[2][2];
    out.rows[1][1] = -mat.rows[0][2] * mat.rows[2][0] + mat.rows[0][0] * mat.rows[2][2];
    out.rows[1][2] =  mat.rows[0][2] * mat.rows[1][0] - mat.rows[0][0] * mat.rows[1][2];
    out.rows[2][0] = -mat.rows[1][1] * mat.rows[2][0] + mat.rows[1][0] * mat.rows[2][1];
    out.rows[2][1] =  mat.rows[0][1] * mat.rows[2][0] - mat.rows[0][0] * mat.rows[2][1];
    out.rows[2][2] = -mat.rows[0][1] * mat.rows[1][0] + mat.rows[0][0] * mat.rows[1][1];

    return out;
}

/**
 * Get the inverse of a matrix
 */
static float4x4 inverse(const float4x4 & mat) {
    float4x4 out;

    float det = determinant(mat);

    // Generated by mathematica
    out.rows[0][0] = (-mat.rows[1][3] * mat.rows[2][2] * mat.rows[3][1] + mat.rows[1][2] * mat.rows[2][3] * mat.rows[3][1] + mat.rows[1][3] * mat.rows[2][1] * mat.rows[3][2] - mat.rows[1][1] * mat.rows[2][3] * mat.rows[3][2] - mat.rows[1][2] * mat.rows[2][1] * mat.rows[3][3] + mat.rows[1][1] * mat.rows[2][2] * mat.rows[3][3]) / det;
    out.rows[0][1] = ( mat.rows[0][3] * mat.rows[2][2] * mat.rows[3][1] - mat.rows[0][2] * mat.rows[2][3] * mat.rows[3][1] - mat.rows[0][3] * mat.rows[2][1] * mat.rows[3][2] + mat.rows[0][1] * mat.rows[2][3] * mat.rows[3][2] + mat.rows[0][2] * mat.rows[2][1] * mat.rows[3][3] - mat.rows[0][1] * mat.rows[2][2] * mat.rows[3][3]) / det;
    out.rows[0][2] = (-mat.rows[0][3] * mat.rows[1][2] * mat.rows[3][1] + mat.rows[0][2] * mat.rows[1][3] * mat.rows[3][1] + mat.rows[0][3] * mat.rows[1][1] * mat.rows[3][2] - mat.rows[0][1] * mat.rows[1][3] * mat.rows[3][2] - mat.rows[0][2] * mat.rows[1][1] * mat.rows[3][3] + mat.rows[0][1] * mat.rows[1][2] * mat.rows[3][3]) / det;
    out.rows[0][3] = ( mat.rows[0][3] * mat.rows[1][2] * mat.rows[2][1] - mat.rows[0][2] * mat.rows[1][3] * mat.rows[2][1] - mat.rows[0][3] * mat.rows[1][1] * mat.rows[2][2] + mat.rows[0][1] * mat.rows[1][3] * mat.rows[2][2] + mat.rows[0][2] * mat.rows[1][1] * mat.rows[2][3] - mat.rows[0][1] * mat.rows[1][2] * mat.rows[2][3]) / det;
    out.rows[1][0] = ( mat.rows[1][3] * mat.rows[2][2] * mat.rows[3][0] - mat.rows[1][2] * mat.rows[2][3] * mat.rows[3][0] - mat.rows[1][3] * mat.rows[2][0] * mat.rows[3][2] + mat.rows[1][0] * mat.rows[2][3] * mat.rows[3][2] + mat.rows[1][2] * mat.rows[2][0] * mat.rows[3][3] - mat.rows[1][0] * mat.rows[2][2] * mat.rows[3][3]) / det;
    out.rows[1][1] = (-mat.rows[0][3] * mat.rows[2][2] * mat.rows[3][0] + mat.rows[0][2] * mat.rows[2][3] * mat.rows[3][0] + mat.rows[0][3] * mat.rows[2][0] * mat.rows[3][2] - mat.rows[0][0] * mat.rows[2][3] * mat.rows[3][2] - mat.rows[0][2] * mat.rows[2][0] * mat.rows[3][3] + mat.rows[0][0] * mat.rows[2][2] * mat.rows[3][3]) / det;
    out.rows[1][2] = ( mat.rows[0][3] * mat.rows[1][2] * mat.rows[3][0] - mat.rows[0][2] * mat.rows[1][3] * mat.rows[3][0] - mat.rows[0][3] * mat.rows[1][0] * mat.rows[3][2] + mat.rows[0][0] * mat.rows[1][3] * mat.rows[3][2] + mat.rows[0][2] * mat.rows[1][0] * mat.rows[3][3] - mat.rows[0][0] * mat.rows[1][2] * mat.rows[3][3]) / det;
    out.rows[1][3] = (-mat.rows[0][3] * mat.rows[1][2] * mat.rows[2][0] + mat.rows[0][2] * mat.rows[1][3] * mat.rows[2][0] + mat.rows[0][3] * mat.rows[1][0] * mat.rows[2][2] - mat.rows[0][0] * mat.rows[1][3] * mat.rows[2][2] - mat.rows[0][2] * mat.rows[1][0] * mat.rows[2][3] + mat.rows[0][0] * mat.rows[1][2] * mat.rows[2][3]) / det;
    out.rows[2][0] = (-mat.rows[1][3] * mat.rows[2][1] * mat.rows[3][0] + mat.rows[1][1] * mat.rows[2][3] * mat.rows[3][0] + mat.rows[1][3] * mat.rows[2][0] * mat.rows[3][1] - mat.rows[1][0] * mat.rows[2][3] * mat.rows[3][1] - mat.rows[1][1] * mat.rows[2][0] * mat.rows[3][3] + mat.rows[1][0] * mat.rows[2][1] * mat.rows[3][3]) / det;
    out.rows[2][1] = ( mat.rows[0][3] * mat.rows[2][1] * mat.rows[3][0] - mat.rows[0][1] * mat.rows[2][3] * mat.rows[3][0] - mat.rows[0][3] * mat.rows[2][0] * mat.rows[3][1] + mat.rows[0][0] * mat.rows[2][3] * mat.rows[3][1] + mat.rows[0][1] * mat.rows[2][0] * mat.rows[3][3] - mat.rows[0][0] * mat.rows[2][1] * mat.rows[3][3]) / det;
    out.rows[2][2] = (-mat.rows[0][3] * mat.rows[1][1] * mat.rows[3][0] + mat.rows[0][1] * mat.rows[1][3] * mat.rows[3][0] + mat.rows[0][3] * mat.rows[1][0] * mat.rows[3][1] - mat.rows[0][0] * mat.rows[1][3] * mat.rows[3][1] - mat.rows[0][1] * mat.rows[1][0] * mat.rows[3][3] + mat.rows[0][0] * mat.rows[1][1] * mat.rows[3][3]) / det;
    out.rows[2][3] = ( mat.rows[0][3] * mat.rows[1][1] * mat.rows[2][0] - mat.rows[0][1] * mat.rows[1][3] * mat.rows[2][0] - mat.rows[0][3] * mat.rows[1][0] * mat.rows[2][1] + mat.rows[0][0] * mat.rows[1][3] * mat.rows[2][1] + mat.rows[0][1] * mat.rows[1][0] * mat.rows[2][3] - mat.rows[0][0] * mat.rows[1][1] * mat.rows[2][3]) / det;
    out.rows[3][0] = ( mat.rows[1][2] * mat.rows[2][1] * mat.rows[3][0] - mat.rows[1][1] * mat.rows[2][2] * mat.rows[3][0] - mat.rows[1][2] * mat.rows[2][0] * mat.rows[3][1] + mat.rows[1][0] * mat.rows[2][2] * mat.rows[3][1] + mat.rows[1][1] * mat.rows[2][0] * mat.rows[3][2] - mat.rows[1][0] * mat.rows[2][1] * mat.rows[3][2]) / det;
    out.rows[3][1] = (-mat.rows[0][2] * mat.rows[2][1] * mat.rows[3][0] + mat.rows[0][1] * mat.rows[2][2] * mat.rows[3][0] + mat.rows[0][2] * mat.rows[2][0] * mat.rows[3][1] - mat.rows[0][0] * mat.rows[2][2] * mat.rows[3][1] - mat.rows[0][1] * mat.rows[2][0] * mat.rows[3][2] + mat.rows[0][0] * mat.rows[2][1] * mat.rows[3][2]) / det;
    out.rows[3][2] = ( mat.rows[0][2] * mat.rows[1][1] * mat.rows[3][0] - mat.rows[0][1] * mat.rows[1][2] * mat.rows[3][0] - mat.rows[0][2] * mat.rows[1][0] * mat.rows[3][1] + mat.rows[0][0] * mat.rows[1][2] * mat.rows[3][1] + mat.rows[0][1] * mat.rows[1][0] * mat.rows[3][2] - mat.rows[0][0] * mat.rows[1][1] * mat.rows[3][2]) / det;
    out.rows[3][3] = (-mat.rows[0][2] * mat.rows[1][1] * mat.rows[2][0] + mat.rows[0][1] * mat.rows[1][2] * mat.rows[2][0] + mat.rows[0][2] * mat.rows[1][0] * mat.rows[2][1] - mat.rows[0][0] * mat.rows[1][2] * mat.rows[2][1] - mat.rows[0][1] * mat.rows[1][0] * mat.rows[2][2] + mat.rows[0][0] * mat.rows[1][1] * mat.rows[2][2]) / det;

    return out;
}

/**
 * Get the upper 3x3 matrix of a 4x4 matrix
 */
static float3x3 upper3x3(const float4x4 & mat) {
    float3x3 out;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            out.rows[i][j] = mat.rows[i][j];

    return out;
}

/**
 * @brief Decompose a matrix constructed from the product translation, rotation, and scale matrices
 * into translation, rotation, and scale vectors.
 */
inline void decompose(const float4x4 & transform, float3 & translation, float3 & rotation, float3 & scale) {
    float4x4 tp = transpose(transform);

    float3 c0 = tp.rows[0].xyz();
    float3 c1 = tp.rows[1].xyz();
    float3 c2 = tp.rows[2].xyz();

    translation = tp.rows[3].xyz();

    scale.x = length(c0);
    scale.y = length(c1);
    scale.z = length(c2);

    c0 = c0 / scale.x;
    c1 = c1 / scale.y;
    c2 = c2 / scale.z;

    rotation.x = atan2f(c1.z, c2.z);
    rotation.y = atan2f(-c0.z, sqrtf(c1.z * c1.z + c2.z * c2.z));
    rotation.z = atan2f(c0.y, c0.x);
}

// TODO: handle NDC -1 to 1 and upside down window coordinates

/**
 * @brief Unproject a point in screen coordinates to world space
 */
inline Ray unproject(const float4x4 & viewProjection, const float2 & pt, const float2 & screenPos, const float2 & screenSz) {
    float2 ndc = (pt - screenPos) / screenSz * 2.0f - 1.0f;
    ndc.y = -ndc.y;

    float4x4 invViewProjection = inverse(viewProjection);

    float4 near(ndc, 0, 1);
    near = invViewProjection * near;
    near = near / near.w;

    float4 far(ndc, 0, 1);
    far = invViewProjection * far;
    far = far / far.w;

    return Ray(near.xyz(), normalize(far.xyz() - near.xyz()));
}

/**
 * @brief Project a point in world space to screen coordinates
 */
inline float2 project(const float4x4 & viewProjection, const float3 & pt, const float2 & screenPos, const float2 & screenSz) {
    float4 ndc = viewProjection * float4(pt, 1);
    ndc = ndc / ndc.w;
    ndc.y = -ndc.y;

    return (ndc.xy() * 0.5 + 0.5) * screenSz + screenPos;
}

#endif
