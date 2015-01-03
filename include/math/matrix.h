/**
 * @file matrix.h
 *
 * @brief Matrix type
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATH_MATRIX_H
#define __MATH_MATRIX_H

#define _USE_MATH_DEFINES
#include <math.h>

#include <math/vector.h>

// M rows x N columns. TODO.
template<typename T, unsigned int M, unsigned int N>
struct matrix {
    T m[M][N];

    // TODO initializer list, and zero()

    matrix() {
        for (unsigned int j = 0; j < M; j++)
            for (unsigned int i = 0; i < N; i++)
                m[j][i] = j == i ? 1 : 0;
    }
};

typedef matrix<float, 3, 3> mat3x3;
typedef matrix<float, 4, 4> mat4x4;

template<typename T, unsigned int M, unsigned int N>
matrix<T, M, N> zero() {
    matrix<T, M, N> mat;

    for (unsigned int j = 0; j < M; j++)
        for (unsigned int i = 0; i < N; i++)
            mat.m[j][i] = 0;

    return mat;
}

template<typename T, unsigned int N>
vector<T, N> operator*(const vector<T, N> & v, const matrix<T, N, N> & m) {
    vector<T, N> result;

    for (unsigned int i = 0; i < N; i++)
        for (unsigned int j = 0; j < N; j++)
            result.v[i] += v.v[i] * m.m[j][i];

    return result;
}

template<typename T, unsigned int N>
vector<T, N> operator*(const matrix<T, N, N> & m, const vector<T, N> & v) {
    vector<T, N> result;

    for (unsigned int i = 0; i < N; i++)
        for (unsigned int j = 0; j < N; j++)
            result.v[i] += m.m[i][j] * v.v[j];

    return result;
}

template<typename T, unsigned int M, unsigned int N, unsigned int P>
matrix<T, M, P> operator*(const matrix<T, M, N> & lhs, const matrix<T, N, P> & rhs) {
    matrix<T, M, P> result = zero<T, M, P>();

    for (unsigned int j = 0; j < M; j++)
        for (unsigned int i = 0; i < P; i++)
            for (unsigned int k = 0; k < N; k++)
                result.m[j][i] += lhs.m[j][k] * rhs.m[k][i];

    return result;
}

template<typename T, unsigned int N>
matrix<T, N, N> transpose(const matrix<T, N, N> & mat) {
    matrix<T, N, N> result; // TODO don't need identityness

    for (unsigned int j = 0; j < N; j++)
        for (unsigned int i = 0; i < N; i++)
            result.m[j][i] = mat.m[i][j];

    return result;
}

/**
 * Get a left handed orthographic projection matrix. TODO linline/static
 */
static mat4x4 orthographicLH(float left, float right, float bottom, float top, float nearz, float farz) {
    mat4x4 out;

    out.m[0][0]  = 2.0f / (right - left);
    out.m[1][1]  = 2.0f / (top - bottom);
    out.m[2][2] = 1.0f / (farz - nearz);
    out.m[3][0] = (left + right) / (left - right);
    out.m[3][1] = (top + bottom) / (bottom - top);
    out.m[3][2] = nearz / (nearz - farz);
    out.m[3][3] = 1.0f;

    return out;
}

/**
 * Get a left handed orthographic projection matrix centered around the origin
 */
static mat4x4 orthographicLH(float width, float height, float depth) {
    return orthographicLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, 0.01f, depth);
}

/**
 * Get a right handed orthographic projection matrix
 */
static mat4x4 orthographicRH(float left, float right, float bottom, float top, float nearz, float farz) {
    mat4x4 out;

    out.m[0][0]  = 2.0f / (right - left);
    out.m[1][1]  = 2.0f / (top - bottom);
    out.m[2][2] = 1.0f / (farz - nearz);
    out.m[3][0] = (left + right) / (left - right);
    out.m[3][1] = (top + bottom) / (bottom - top);
    out.m[3][2] = nearz / (nearz - farz);
    out.m[3][3] = 1.0f;

    return out;
}

/**
 * Get a right handed orthographic projection matrix centered around the origin
 */
static mat4x4 orthographicRH(float width, float height, float depth) {
    return orthographicRH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, -depth / 2.0f, depth / 2.0f);
}

/**
 * Create a left handed look-at matrix
 */
static mat4x4 lookAtLH(const vec3 & position, const vec3 & target, const vec3 & up) {
    vec3 zAxis = normalize(target - position);
    vec3 xAxis = normalize(cross(up, zAxis));
    vec3 yAxis = normalize(cross(zAxis, xAxis));

    mat4x4 out;

    out.m[0][0] = xAxis.x;
    out.m[0][1] = yAxis.x;
    out.m[0][2] = zAxis.x;
    out.m[0][3] = 0;

    out.m[1][0] = xAxis.y;
    out.m[1][1] = yAxis.y;
    out.m[1][2] = zAxis.y;
    out.m[1][3] = 0;

    out.m[2][0] = xAxis.z;
    out.m[2][1] = yAxis.z;
    out.m[2][2] = zAxis.z;
    out.m[2][3] = 0;

    out.m[3][0] = dot(-xAxis, position);
    out.m[3][1] = dot(-yAxis, position);
    out.m[3][2] = dot(-zAxis, position);
    out.m[3][3] = 1;

    return out;
}

/**
 * Create a right handed look-at matrix
 */
static mat4x4 lookAtRH(const vec3 & position, const vec3 & target, const vec3 & up) {
    vec3 zAxis = normalize(target - position);
    vec3 xAxis = normalize(cross(up, zAxis));
    vec3 yAxis = cross(zAxis, xAxis);

    mat4x4 out;

    out.m[0][0] = xAxis.x;
    out.m[1][0] = yAxis.x;
    out.m[2][0] = zAxis.x;
    out.m[3][0] = 0;

    out.m[0][1] = xAxis.y;
    out.m[1][1] = yAxis.y;
    out.m[2][1] = zAxis.y;
    out.m[3][1] = 0;

    out.m[0][2] = xAxis.z;
    out.m[1][2] = yAxis.z;
    out.m[2][2] = zAxis.z;
    out.m[3][2] = 0;

    out.m[0][3] = dot(xAxis, position);
    out.m[1][3] = dot(yAxis, position);
    out.m[2][3] = dot(zAxis, position);
    out.m[3][3] = 1;

    return out;
}

/**
 * Create a left handed perspective projection matrix
 */
static mat4x4 perspectiveLH(float fov, float aspect, float znear, float zfar) {
    float h = cos(fov / 2.0f) / sin(fov / 2.0f);
    float w = h / aspect;

    mat4x4 out;

    out.m[0][0] = w;
    out.m[1][1] = h;
    out.m[2][2] = zfar / (zfar - znear);
    out.m[2][3] = -znear * zfar / (zfar - znear);
    out.m[3][2] = 1.0f;

    return out;
}

/**
 * Create a right handed perspective projection matrix
 */
static mat4x4 perspectiveRH(float fov, float aspect, float znear, float zfar) {
    float h = cos(fov / 2.0f) / sin(fov / 2.0f);
    float w = h / aspect;

    mat4x4 out;

    out.m[0][0] = w;
    out.m[1][1] = h;
    out.m[2][2] = zfar / (zfar - znear);
    out.m[2][3] = znear * zfar / (zfar - znear);
    out.m[3][2] = -1.0f;

    return out;
}

/**
 * Get a rotation matrix around the X axis
 */
static mat4x4 rotationX(float radians) {
    float cost = cosf(radians);
    float sint = sinf(radians);

    mat4x4 out;

    out.m[0][0] = 1.0f;
    out.m[1][1] = cost;
    out.m[1][2] = -sint;
    out.m[2][1] = sint;
    out.m[2][2] = cost;
    out.m[3][3] = 1.0f;

    return out;
}

/**
 * Get a rotation matrix around the Y axis
 */
static mat4x4 rotationY(float radians) {
    float cost = cosf(radians);
    float sint = sinf(radians);

    mat4x4 out;

    out.m[0][0] = cost;
    out.m[0][2] = sint;
    out.m[1][1] = 1.0f;
    out.m[2][0] = -sint;
    out.m[2][2] = cost;
    out.m[3][3] = 1.0f;

    return out;
}

/**
 * Get a rotation matrix around the Z axis
 */
static mat4x4 rotationZ(float radians) {
    float cost = cosf(radians);
    float sint = sinf(radians);

    mat4x4 out;

    out.m[0][0] = cost;
    out.m[0][1] = -sint;
    out.m[1][0] = sint;
    out.m[1][1] = cost;
    out.m[2][2] = 1.0f;
    out.m[3][3] = 1.0f;

    return out;
}

/**
 * Create a yaw, pitch, roll rotation matrix. The operations are applied
 * in the order: roll, then pitch, then yaw, around the origin
 */
static mat4x4 yawPitchRoll(float yaw, float pitch, float roll) {
    // return rotationY(yaw) * rotationX(pitch) * rotationZ(roll);

    mat4x4 out;

    float cy = cosf(yaw);
    float sy = sinf(yaw);
    float cp = cosf(pitch);
    float sp = sinf(pitch);
    float cr = cosf(roll);
    float sr = sinf(roll);

    // Generated by mathematica
    out.m[0][0] = cr * cy + sp * sr * sy;
    out.m[0][1] = -cy * sr + cr * sp * sy;
    out.m[0][2] = cp * sy;
    out.m[0][3] = 0.0f;
    out.m[1][0] = cp * sr;
    out.m[1][1] = cp * cr;
    out.m[1][2] = -sp;
    out.m[1][3] = 0.0f;
    out.m[2][0] = cy * sp * sr - cr * sy;
    out.m[2][1] = cr * cy * sp + sr * sy;
    out.m[2][2] = cp * cy;
    out.m[2][3] = 0.0f;
    out.m[3][0] = 0.0f;
    out.m[3][1] = 0.0f;
    out.m[3][2] = 0.0f;
    out.m[3][3] = 1.0f;

    return out;
}

/**
 * Get a scaling matrix
 */
static mat4x4 scale(float x, float y, float z) {
    mat4x4 out;

    out.m[0][0] = x;
    out.m[1][1] = y;
    out.m[2][2] = z;
    out.m[3][3] = 1.0f;

    return out;
}

/**
 * Get a translation matrix
 */
static mat4x4 translation(float x, float y, float z) {
    mat4x4 out;

    out.m[0][0] = 1.0f;
    out.m[0][3] = x;
    out.m[1][1] = 1.0f;
    out.m[1][3] = y;
    out.m[2][2] = 1.0f;
    out.m[2][3] = z;
    out.m[3][3] = 1.0f;

    return out;
}

/**
 * Get the determinant of a matrix
 */
static float determinant(const mat4x4 & mat) {
    // Generated by mathematica
    return mat.m[0][3] * mat.m[1][2] * mat.m[2][1] * mat.m[3][0] - mat.m[0][2] * mat.m[1][3] * mat.m[2][1] * mat.m[3][0] - mat.m[0][3] * mat.m[1][1] * mat.m[2][2] * mat.m[3][0] +
           mat.m[0][1] * mat.m[1][3] * mat.m[2][2] * mat.m[3][0] + mat.m[0][2] * mat.m[1][1] * mat.m[2][3] * mat.m[3][0] - mat.m[0][1] * mat.m[1][2] * mat.m[2][3] * mat.m[3][0] -
           mat.m[0][3] * mat.m[1][2] * mat.m[2][0] * mat.m[3][1] + mat.m[0][2] * mat.m[1][3] * mat.m[2][0] * mat.m[3][1] + mat.m[0][3] * mat.m[1][0] * mat.m[2][2] * mat.m[3][1] -
           mat.m[0][0] * mat.m[1][3] * mat.m[2][2] * mat.m[3][1] - mat.m[0][2] * mat.m[1][0] * mat.m[2][3] * mat.m[3][1] + mat.m[0][0] * mat.m[1][2] * mat.m[2][3] * mat.m[3][1] +
           mat.m[0][3] * mat.m[1][1] * mat.m[2][0] * mat.m[3][2] - mat.m[0][1] * mat.m[1][3] * mat.m[2][0] * mat.m[3][2] - mat.m[0][3] * mat.m[1][0] * mat.m[2][1] * mat.m[3][2] +
           mat.m[0][0] * mat.m[1][3] * mat.m[2][1] * mat.m[3][2] + mat.m[0][1] * mat.m[1][0] * mat.m[2][3] * mat.m[3][2] - mat.m[0][0] * mat.m[1][1] * mat.m[2][3] * mat.m[3][2] -
           mat.m[0][2] * mat.m[1][1] * mat.m[2][0] * mat.m[3][3] + mat.m[0][1] * mat.m[1][2] * mat.m[2][0] * mat.m[3][3] + mat.m[0][2] * mat.m[1][0] * mat.m[2][1] * mat.m[3][3] -
           mat.m[0][0] * mat.m[1][2] * mat.m[2][1] * mat.m[3][3] - mat.m[0][1] * mat.m[1][0] * mat.m[2][2] * mat.m[3][3] + mat.m[0][0] * mat.m[1][1] * mat.m[2][2] * mat.m[3][3];
}

/**
 * Get the inverse of a matrix
 */
static mat4x4 inverse(const mat4x4 & mat) {
    mat4x4 out;

    float det = determinant(mat);

    // Generated by mathematica
    out.m[0][0] = (-mat.m[1][3] * mat.m[2][2] * mat.m[3][1] + mat.m[1][2] * mat.m[2][3] * mat.m[3][1] + mat.m[1][3] * mat.m[2][1] * mat.m[3][2] - mat.m[1][1] * mat.m[2][3] * mat.m[3][2] - mat.m[1][2] * mat.m[2][1] * mat.m[3][3] + mat.m[1][1] * mat.m[2][2] * mat.m[3][3]) / det;
    out.m[0][1] = ( mat.m[0][3] * mat.m[2][2] * mat.m[3][1] - mat.m[0][2] * mat.m[2][3] * mat.m[3][1] - mat.m[0][3] * mat.m[2][1] * mat.m[3][2] + mat.m[0][1] * mat.m[2][3] * mat.m[3][2] + mat.m[0][2] * mat.m[2][1] * mat.m[3][3] - mat.m[0][1] * mat.m[2][2] * mat.m[3][3]) / det;
    out.m[0][2] = (-mat.m[0][3] * mat.m[1][2] * mat.m[3][1] + mat.m[0][2] * mat.m[1][3] * mat.m[3][1] + mat.m[0][3] * mat.m[1][1] * mat.m[3][2] - mat.m[0][1] * mat.m[1][3] * mat.m[3][2] - mat.m[0][2] * mat.m[1][1] * mat.m[3][3] + mat.m[0][1] * mat.m[1][2] * mat.m[3][3]) / det;
    out.m[0][3] = ( mat.m[0][3] * mat.m[1][2] * mat.m[2][1] - mat.m[0][2] * mat.m[1][3] * mat.m[2][1] - mat.m[0][3] * mat.m[1][1] * mat.m[2][2] + mat.m[0][1] * mat.m[1][3] * mat.m[2][2] + mat.m[0][2] * mat.m[1][1] * mat.m[2][3] - mat.m[0][1] * mat.m[1][2] * mat.m[2][3]) / det;
    out.m[1][0] = ( mat.m[1][3] * mat.m[2][2] * mat.m[3][0] - mat.m[1][2] * mat.m[2][3] * mat.m[3][0] - mat.m[1][3] * mat.m[2][0] * mat.m[3][2] + mat.m[1][0] * mat.m[2][3] * mat.m[3][2] + mat.m[1][2] * mat.m[2][0] * mat.m[3][3] - mat.m[1][0] * mat.m[2][2] * mat.m[3][3]) / det;
    out.m[1][1] = (-mat.m[0][3] * mat.m[2][2] * mat.m[3][0] + mat.m[0][2] * mat.m[2][3] * mat.m[3][0] + mat.m[0][3] * mat.m[2][0] * mat.m[3][2] - mat.m[0][0] * mat.m[2][3] * mat.m[3][2] - mat.m[0][2] * mat.m[2][0] * mat.m[3][3] + mat.m[0][0] * mat.m[2][2] * mat.m[3][3]) / det;
    out.m[1][2] = ( mat.m[0][3] * mat.m[1][2] * mat.m[3][0] - mat.m[0][2] * mat.m[1][3] * mat.m[3][0] - mat.m[0][3] * mat.m[1][0] * mat.m[3][2] + mat.m[0][0] * mat.m[1][3] * mat.m[3][2] + mat.m[0][2] * mat.m[1][0] * mat.m[3][3] - mat.m[0][0] * mat.m[1][2] * mat.m[3][3]) / det;
    out.m[1][3] = (-mat.m[0][3] * mat.m[1][2] * mat.m[2][0] + mat.m[0][2] * mat.m[1][3] * mat.m[2][0] + mat.m[0][3] * mat.m[1][0] * mat.m[2][2] - mat.m[0][0] * mat.m[1][3] * mat.m[2][2] - mat.m[0][2] * mat.m[1][0] * mat.m[2][3] + mat.m[0][0] * mat.m[1][2] * mat.m[2][3]) / det;
    out.m[2][0] = (-mat.m[1][3] * mat.m[2][1] * mat.m[3][0] + mat.m[1][1] * mat.m[2][3] * mat.m[3][0] + mat.m[1][3] * mat.m[2][0] * mat.m[3][1] - mat.m[1][0] * mat.m[2][3] * mat.m[3][1] - mat.m[1][1] * mat.m[2][0] * mat.m[3][3] + mat.m[1][0] * mat.m[2][1] * mat.m[3][3]) / det;
    out.m[2][1] = ( mat.m[0][3] * mat.m[2][1] * mat.m[3][0] - mat.m[0][1] * mat.m[2][3] * mat.m[3][0] - mat.m[0][3] * mat.m[2][0] * mat.m[3][1] + mat.m[0][0] * mat.m[2][3] * mat.m[3][1] + mat.m[0][1] * mat.m[2][0] * mat.m[3][3] - mat.m[0][0] * mat.m[2][1] * mat.m[3][3]) / det;
    out.m[2][2] = (-mat.m[0][3] * mat.m[1][1] * mat.m[3][0] + mat.m[0][1] * mat.m[1][3] * mat.m[3][0] + mat.m[0][3] * mat.m[1][0] * mat.m[3][1] - mat.m[0][0] * mat.m[1][3] * mat.m[3][1] - mat.m[0][1] * mat.m[1][0] * mat.m[3][3] + mat.m[0][0] * mat.m[1][1] * mat.m[3][3]) / det;
    out.m[2][3] = ( mat.m[0][3] * mat.m[1][1] * mat.m[2][0] - mat.m[0][1] * mat.m[1][3] * mat.m[2][0] - mat.m[0][3] * mat.m[1][0] * mat.m[2][1] + mat.m[0][0] * mat.m[1][3] * mat.m[2][1] + mat.m[0][1] * mat.m[1][0] * mat.m[2][3] - mat.m[0][0] * mat.m[1][1] * mat.m[2][3]) / det;
    out.m[3][0] = ( mat.m[1][2] * mat.m[2][1] * mat.m[3][0] - mat.m[1][1] * mat.m[2][2] * mat.m[3][0] - mat.m[1][2] * mat.m[2][0] * mat.m[3][1] + mat.m[1][0] * mat.m[2][2] * mat.m[3][1] + mat.m[1][1] * mat.m[2][0] * mat.m[3][2] - mat.m[1][0] * mat.m[2][1] * mat.m[3][2]) / det;
    out.m[3][1] = (-mat.m[0][2] * mat.m[2][1] * mat.m[3][0] + mat.m[0][1] * mat.m[2][2] * mat.m[3][0] + mat.m[0][2] * mat.m[2][0] * mat.m[3][1] - mat.m[0][0] * mat.m[2][2] * mat.m[3][1] - mat.m[0][1] * mat.m[2][0] * mat.m[3][2] + mat.m[0][0] * mat.m[2][1] * mat.m[3][2]) / det;
    out.m[3][2] = ( mat.m[0][2] * mat.m[1][1] * mat.m[3][0] - mat.m[0][1] * mat.m[1][2] * mat.m[3][0] - mat.m[0][2] * mat.m[1][0] * mat.m[3][1] + mat.m[0][0] * mat.m[1][2] * mat.m[3][1] + mat.m[0][1] * mat.m[1][0] * mat.m[3][2] - mat.m[0][0] * mat.m[1][1] * mat.m[3][2]) / det;
    out.m[3][3] = (-mat.m[0][2] * mat.m[1][1] * mat.m[2][0] + mat.m[0][1] * mat.m[1][2] * mat.m[2][0] + mat.m[0][2] * mat.m[1][0] * mat.m[2][1] - mat.m[0][0] * mat.m[1][2] * mat.m[2][1] - mat.m[0][1] * mat.m[1][0] * mat.m[2][2] + mat.m[0][0] * mat.m[1][1] * mat.m[2][2]) / det;

    return out;
}

#endif
