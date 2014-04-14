/**
 * @file rtmath.h
 *
 * @brief Raytracer 3D math library
 *
 * @author Sean James
 */

#ifndef _RTMATH_H
#define _RTMATH_H

#include <defs.h>

#define MIN2(x, y) ((x) < (y) ? (x) : (y))
#define MAX2(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(x, min, max) (MAX2(MIN2(x, max), min))
#define SATURATE(x) (CLAMP(x, 0.0f, 1.0f))
#define SIGN(x) ((x) > 0 ? 1 : -1)
#define SIGNF(x) ((x) > 0.0f ? 1.0f : -1.0f)
#define INFINITY32F ((float)0x7F800000)

/**
 * A 32 bit floating point vector with 2 components: X and Y
 */
struct Vec2 {

    float x; // X component
    float y; // Y component

    /**
     * Constructor accepts X and Y values
     */
    inline Vec2(float X, float Y)
        : x(X), y(Y)
    {
    }

    /**
     * Empty constructor initializes all components to 0
     */
    inline Vec2()
        : x(0.0f), y(0.0f)
    {
    }

    /**
     * Take the dot product of this vector with another vector
     */
    inline float dot(Vec2 other) {
        return x * other.x + y * other.y;
    }

    /**
     * Get the squared length of this vector
     */
    inline float len2() {
        return x * x + y * y;
    }

    /**
     * Get the length of this vector
     */
    inline float len() {
        return sqrtf(x * x + y * y);
    }

    /**
     * Add two vectors component wise
     */
    inline Vec2 operator+(Vec2 other) {
        return Vec2(x + other.x, y + other.y);
    }

    /**
     * Subtract a vector from another component wise
     */
    inline Vec2 operator-(Vec2 other) {
        return Vec2(x - other.x, y - other.y);
    }

    /**
     * Negate each component of a vector
     */
    inline Vec2 operator-() {
        return Vec2(-x, -y);
    }

    /**
     * Multiply two vectors component wise
     */
    inline Vec2 operator*(Vec2 other) {
        return Vec2(x * other.x, y * other.y);
    }

    /**
     * Divide two vectors component wise
     */
    inline Vec2 operator/(Vec2 other) {
        return Vec2(x / other.x, y / other.y);
    }

    /**
     * Multiply each component of a vector by a constant
     */
    inline Vec2 operator*(float c) {
        return Vec2(x * c, y * c);
    }

    /**
     * Divide each component of a vector by a constant
     */
    inline Vec2 operator/(float c) {
        return Vec2(x / c, y / c);
    }

    /**
     * Add a constant to each component
     */
    inline Vec2 operator+(float c) {
        return Vec2(x + c, y + c);
    }

    /**
     * Subtract a constant from each component
     */
    inline Vec2 operator-(float c) {
        return Vec2(x - c, y - c);
    }

    /**
     * Add a Vec2 to this Vec2
     */
    inline Vec2 operator+=(Vec2 other) {
        x += other.x;
        y += other.y;

        return *this;
    }

    /**
     * Scale the length of the vector to be 1
     */
    inline void normalize() {
        float len = sqrtf(x * x + y * y);

        x /= len;
        y /= len;
    }

    /**
     * Check whether the components of two vectors are equal
     */
    inline bool operator== (Vec2 other) {
        return x == other.x && y == other.y;
    }

};

/**
 * A 32 bit floating point vector with 3 components: X, Y, and Z
 */
struct Vec3 {

    float x; // X component
    float y; // Y component
    float z; // Z component

    // TODO: check
    inline void set(int component, float value) {
        //*(&x + component) = value;

        switch(component) {
        case 0:
            x = value;
            break;
        case 1:
            y = value;
            break;
        case 2:
            z = value;
            break;
        }
    }

    inline float get(int component) {
        //return *(&x + component);

        switch(component) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        }
        
        return 0.0f;
    }

    /**
     * Constructor accepts X, Y, and Z values
     */
    inline Vec3(float X, float Y, float Z)
        : x(X), y(Y), z(Z)
    {
    }

    /**
     * Empty constructor initializes all components to 0
     */
    inline Vec3()
        : x(0.0f), y(0.0f), z(0.0f)
    {
    }

    /**
     * Take the dot product of this vector with another vector
     */
    inline float dot(Vec3 other) {
        return x * other.x + y * other.y + z * other.z;
    }

    /**
     * Get the squared length of this vector
     */
    inline float len2() {
        return x * x + y * y + z * z;
    }

    /**
     * Get the length of this vector
     */
    inline float len() {
        return sqrtf(x * x + y * y + z * z);
    }

    /**
     * Add two vectors component wise
     */
    inline Vec3 operator+(Vec3 other) {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    /**
     * Subtract a vector from another component wise
     */
    inline Vec3 operator-(Vec3 other) {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    /**
     * Negate each component of a vector
     */
    inline Vec3 operator-() {
        return Vec3(-x, -y, -z);
    }

    /**
     * Multiply two vectors component wise
     */
    inline Vec3 operator*(Vec3 other) {
        return Vec3(x * other.x, y * other.y, z * other.z);
    }

    /**
     * Divide two vectors component wise
     */
    inline Vec3 operator/(Vec3 other) {
        return Vec3(x / other.x, y / other.y, z / other.z);
    }

    /**
     * Multiply each component of a vector by a constant
     */
    inline Vec3 operator*(float c) {
        return Vec3(x * c, y * c, z * c);
    }

    /**
     * Divide each component of a vector by a constant
     */
    inline Vec3 operator/(float c) {
        return Vec3(x / c, y / c, z / c);
    }

    /**
     * Add a constant to each component
     */
    inline Vec3 operator+(float c) {
        return Vec3(x + c, y + c, z + c);
    }

    /**
     * Subtract a constant from each component
     */
    inline Vec3 operator-(float c) {
        return Vec3(x - c, y - c, z - c);
    }

    /**
     * Add a Vec3 to this Vec3
     */
    inline Vec3 operator+=(Vec3 other) {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }

    /**
     * Scale the length of the vector to be 1
     */
    inline void normalize() {
        float len = sqrtf(x * x + y * y + z * z);

        x /= len;
        y /= len;
        z /= len;
    }

    /**
     * Check whether the components of two vectors are equal
     */
    inline bool operator== (Vec3 other) {
        return x == other.x && y == other.y && z == other.z;
    }

    /**
     * Reflect this vector across a normal vector
     */
    inline Vec3 reflect(Vec3 across) {
        float amt = -2.0f * dot(across);
        Vec3 b = across;
        return *this + b * amt;
    }

    /**
     * Get a refracted vector according to Snell's law. Assumes that this vector points towards
     * the interface.
     *
     * @param norm Normal at interface
     * @param n1   Index of refraction of material being left
     * @param n2   Index of refractio nof material being entered
     */
    inline Vec3 refract(Vec3 norm, float n1, float n2) {
        Vec3 L = -*this;
        Vec3 N = norm;

        float r = n1 / n2;
        float cos_theta_l = L.dot(N);
        float c = sqrtf(1.0f - r * r * (1.0f - cos_theta_l * cos_theta_l));

        float c_l = -r;
        float c_n = -c - r * cos_theta_l;

        return L * c_l + N * c_n;

        /*float n = n1 / n2;
        float c1 = dot(across);
        float c2 = sqrtf(1.0f - n * n * (1.0f - c1 * c1));
        float b = (n * c1 - c2);
        Vec3 c = across;
        return *this * n + c * b;*/
    }

    /**
     * @brief Schlick's approximation
     *
     * @param n  Normal
     * @param v  View direction
     * @param n1 Index of refraction of material being left
     * @param n2 Index of refraction of material being entered
     *
     * @return A factor for blending reflection and refraction. 0 = refraction only,
     * 1 = reflection only
     */
    static float schlick(Vec3 n, Vec3 v, float n1, float n2) {
        // TODO specular highlights can use this too?

        float cos_i = n.dot(v);

        float r0 = (n1 - n2) / (n1 + n2);
        r0 *= r0;

        // TODO: total internal reflection

        return r0 + (1.0f - r0) * pow(1.0f - cos_i, 5.0f);
    }

    /**
     * Get the cross product of this vector and another
     */
    inline Vec3 cross(Vec3 other) {
        return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
    }

};

/**
 * A 32 bit floating point vector with 4 components: X, Y, Z, and W
 */
struct Vec4 {

    float x; // X component
    float y; // Y component
    float z; // Z component
    float w; // W component

    /**
     * Constructor accepts X, Y, Z, and W values
     */
    inline Vec4(float X, float Y, float Z, float W)
        : x(X), y(Y), z(Z), w(W)
    {
    }

    /**
     * Constructor accepting a Vec3 and a W value
     */
    inline Vec4(Vec3 xyz, float w)
        : x(xyz.x), y(xyz.y), z(xyz.z), w(w)
    {
    }

    /**
     * Empty constructor initializes all components to 0
     */
    inline Vec4()
        : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
    {
    }

    /**
     * Take the dot product of this vector with another vector
     */
    inline float dot(Vec4 other) {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    /**
     * Get the squared length of this vector
     */
    inline float len2() {
        return x * x + y * y + z * z + w * w;
    }

    /**
     * Get the length of this vector
     */
    inline float len() {
        return sqrtf(x * x + y * y + z * z + w * w);
    }

    /**
     * Add two vectors component wise
     */
    inline Vec4 operator+(Vec4 other) {
        return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    /**
     * Subtract a vector from another component wise
     */
    inline Vec4 operator-(Vec4 other) {
        return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    /**
     * Negate each component of a vector
     */
    inline Vec4 operator-() {
        return Vec4(-x, -y, -z, -w);
    }

    /**
     * Multiply two vectors component wise
     */
    inline Vec4 operator*(Vec4 other) {
        return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    /**
     * Divide two vectors component wise
     */
    inline Vec4 operator/(Vec4 other) {
        return Vec4(x / other.x, y / other.y, z / other.z, w / other.w);
    }

    /**
     * Multiply each component of a vector by a constant
     */
    inline Vec4 operator*(float c) {
        return Vec4(x * c, y * c, z * c, w * c);
    }

    /**
     * Divide each component of a vector by a constant
     */
    inline Vec4 operator/(float c) {
        return Vec4(x / c, y / c, z / c, w / c);
    }

    /**
     * Add a constant to each component
     */
    inline Vec4 operator+(float c) {
        return Vec4(x + c, y + c, z + c, w + c);
    }

    /**
     * Subtract a constant from each component
     */
    inline Vec4 operator-(float c) {
        return Vec4(x - c, y - c, z - c, w - c);
    }

    /**
     * Add a Vec4 to this Vec4
     */
    inline Vec4 operator+=(Vec4 other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;

        return *this;
    }

    /**
     * Scale the length of the vector to be 1
     */
    inline void normalize() {
        float len = sqrtf(x * x + y * y + z * z + w * w);

        x /= len;
        y /= len;
        z /= len;
        w /= len;
    }

    /**
     * Check whether the components of two vectors are equal
     */
    inline bool operator== (Vec4 other) {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

};

/**
 * A 4 row x 4 column 32 bit floating point matrix.
 * 
 * Matrix components are stored in a row major order. Thus vector operations 
 * treat vectors as column vectors and matrices are pre-multiplied.
 */
struct Mat4x4 {

    float m[16];  // Elements of the matrix

    /**
     * Empty constructor creates an identity matrix
     */
    Mat4x4()
    {
        m[0]  = 1.0f;
        m[1]  = 0.0f;
        m[2]  = 0.0f;
        m[3]  = 0.0f;

        m[4]  = 0.0f;
        m[5]  = 1.0f;
        m[6]  = 0.0f;
        m[7]  = 0.0f;

        m[8]  = 0.0f;
        m[9]  = 0.0f;
        m[10] = 1.0f;
        m[11] = 0.0f;

        m[12] = 0.0f;
        m[13] = 0.0f;
        m[14] = 0.0f;
        m[15] = 1.0f;
    }

    /**
     * Constructor specifying each element of the matrix
     */
    Mat4x4(float m00, float m01, float m02, float m03,
           float m10, float m11, float m12, float m13,
           float m20, float m21, float m22, float m23,
           float m30, float m31, float m32, float m33)
    {
        m[0]  = m00;
        m[1]  = m01;
        m[2]  = m02;
        m[3]  = m03;

        m[4]  = m10;
        m[5]  = m11;
        m[6]  = m12;
        m[7]  = m13;

        m[8]  = m20;
        m[9]  = m21;
        m[10] = m22;
        m[11] = m23;

        m[12] = m30;
        m[13] = m31;
        m[14] = m32;
        m[15] = m33;
    }

    /**
     * Transform a Vec2 by this matrix. Assumes the third and fourth components
     * are 0 and 1 respectively.
     */
    Vec2 operator* (Vec2 v) {
        return Vec2(
            m[0] * v.x + m[1] * v.y + m[3],
            m[4] * v.x + m[5] * v.y + m[7]);
    }

    /**
     * Transform a Vec2 by this matrix. Assumes the third and fourth components
     * are 0 and 1 respectively.
     */
    Vec2 transform(Vec2 v) {
        return *this * v;
    }

    /**
     * Transform a Vec3 by this matrix. Assumes the fourth component is a 1.
     */
    Vec3 operator* (Vec3 v) {
        return Vec3(
            m[0] * v.x + m[1] * v.y + m[2]  * v.z + m[3],
            m[4] * v.x + m[5] * v.y + m[6]  * v.z + m[7],
            m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11]);
    }

    /**
     * Transform a Vec3 by this matrix. Assumes the fourth component is a 1.
     */
    Vec3 transform(Vec3 v) {
        return *this * v;
    }

    /**
     * Transform a Vec4 by this matrix
     */
    Vec4 operator* (Vec4 v) {
        return Vec4(
            m[0]  * v.x + m[1]  * v.y + m[2]  * v.z + m[3]  * v.w,
            m[4]  * v.x + m[5]  * v.y + m[6]  * v.z + m[7]  * v.w,
            m[8]  * v.x + m[9]  * v.y + m[10] * v.z + m[11] * v.w,
            m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w);
    }

    /**
     * Transform a Vec4 by this matrix
     */
    Vec4 transform(Vec4 v) {
        return *this * v;
    }

    /**
     * Multiply two matrices together
     */
    Mat4x4 operator* (Mat4x4 b) {
        Mat4x4 out;

        // TODO make this cache friendly
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                float r = 0.0f;

                for (int k = 0; k < 4; k++)
                    r += m[i * 4 + k] * b.m[k * 4 + j];

                out.m[i * 4 + j] = r;
            }
        }

        return out;
    }

    /**
     * Get a 4x4 identity matrix
     */
    static Mat4x4 identity() {
        Mat4x4 out;

        return out;
    }

    /**
     * Get a 4x4 zero matrix
     */
    static Mat4x4 zero() {
        Mat4x4 out;

        for (int i = 0; i < 16; i++)
            out.m[i] = 0;

        return out;
    }

    /**
     * Get a left handed orthographic projection matrix
     */
    static Mat4x4 orthographicLH(float left, float right, float bottom, float top, float nearz, float farz) {
        Mat4x4 out;

        out.m[0]  = 2.0f / (right - left);
        out.m[5]  = 2.0f / (top - bottom);
        out.m[10] = 1.0f / (farz - nearz);
        out.m[12] = (left + right) / (left - right);
        out.m[13] = (top + bottom) / (bottom - top);
        out.m[14] = nearz / (nearz - farz);
        out.m[15] = 1.0f;
        
        return out;
    }

    /**
     * Get a left handed orthographic projection matrix centered around the origin
     */
    static Mat4x4 orthographicLH(float width, float height, float depth) {
        return orthographicLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, 0.01f, depth);
    }

    /**
     * Get a right handed orthographic projection matrix
     */
    static Mat4x4 orthographicRH(float left, float right, float bottom, float top, float nearz, float farz) {
        Mat4x4 out;

        out.m[0]  = 2.0f / (right - left);
        out.m[5]  = 2.0f / (top - bottom);
        out.m[10] = 1.0f / (farz - nearz);
        out.m[12] = (left + right) / (left - right);
        out.m[13] = (top + bottom) / (bottom - top);
        out.m[14] = nearz / (nearz - farz);
        out.m[15] = 1.0f;

        return out;
    }

    /**
     * Get a right handed orthographic projection matrix centered around the origin
     */
    static Mat4x4 orthographicRH(float width, float height, float depth) {
        return orthographicRH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, -depth / 2.0f, depth / 2.0f);
    }

    /**
     * Create a left handed look-at matrix
     */
    static Mat4x4 lookAtLH(Vec3 position, Vec3 target, Vec3 up) {
        Vec3 zAxis = target - position;
        zAxis.normalize();

        Vec3 xAxis = up.cross(zAxis);
        xAxis.normalize();

        Vec3 yAxis = zAxis.cross(xAxis);
        yAxis.normalize();

        Mat4x4 out;

        out.m[0]  = xAxis.x;
        out.m[1]  = yAxis.x;
        out.m[2]  = zAxis.x;
        out.m[3]  = 0;

        out.m[4]  = xAxis.y;
        out.m[5]  = yAxis.y;
        out.m[6]  = zAxis.y;
        out.m[7]  = 0;

        out.m[8]  = xAxis.z;
        out.m[9]  = yAxis.z;
        out.m[10] = zAxis.z;
        out.m[11] = 0;

        out.m[12] = -xAxis.dot(position);
        out.m[13] = -yAxis.dot(position);
        out.m[14] = -zAxis.dot(position);
        out.m[15] = 1;

        return out;
    }

    /**
     * Create a right handed look-at matrix
     */
    static Mat4x4 lookAtRH(Vec3 position, Vec3 target, Vec3 up) {
        Vec3 zAxis = target - position;
        zAxis.normalize();

        Vec3 xAxis = up.cross(zAxis);
        xAxis.normalize();

        Vec3 yAxis = zAxis.cross(xAxis);

        Mat4x4 out;

        out.m[0]  = xAxis.x;
        out.m[4]  = yAxis.x;
        out.m[8]  = zAxis.x;
        out.m[12]  = 0;

        out.m[1]  = xAxis.y;
        out.m[5]  = yAxis.y;
        out.m[9]  = zAxis.y;
        out.m[13]  = 0;

        out.m[2]  = xAxis.z;
        out.m[6]  = yAxis.z;
        out.m[10] = zAxis.z;
        out.m[14] = 0;

        out.m[3] = xAxis.dot(position);
        out.m[7] = yAxis.dot(position);
        out.m[11] = zAxis.dot(position);
        out.m[15] = 1;

        return out;
    }

    /**
     * Create a left handed perspective projection matrix
     */
    static Mat4x4 perspectiveLH(float fov, float aspect, float znear, float zfar) {
        float h = cos(fov / 2.0f) / sin(fov / 2.0f);
        float w = h / aspect;

        Mat4x4 out;

        out.m[0] = w;
        out.m[5] = h;
        out.m[10] = zfar / (zfar - znear);
        out.m[14] = 1.0f;
        out.m[11] = -znear * zfar / (zfar - znear);

        return out;
    }

    /**
     * Create a right handed perspective projection matrix
     */
    static Mat4x4 perspectiveRH(float fov, float aspect, float znear, float zfar) {
        float h = cos(fov / 2.0f) / sin(fov / 2.0f);
        float w = h / aspect;

        Mat4x4 out;

        out.m[0] = w;
        out.m[5] = h;
        out.m[10] = zfar / (zfar - znear);
        out.m[14] = -1.0f;
        out.m[11] = znear * zfar / (zfar - znear);

        return out;
    }

    /**
     * Get the transpose of a matrix
     */
    static Mat4x4 transpose(Mat4x4 mat) {
        Mat4x4 out;

        for (int j = 0; j < 4; j++)
            for (int i = 0; i < 4; i++)
                out.m[j * 4 + i] = mat.m[i * 4 + j];

        return out;
    }

    /**
     * Get a rotation matrix around the X axis
     */
    static Mat4x4 rotationX(float radians) {
        float cost = cosf(radians);
        float sint = sinf(radians);

        Mat4x4 out;

        out.m[0] = 1.0f;
        out.m[5] = cost;
        out.m[6] = -sint;
        out.m[9] = sint;
        out.m[10] = cost;
        out.m[15] = 1.0f;

        return out;
    }

    /**
     * Get a rotation matrix around the Y axis
     */
    static Mat4x4 rotationY(float radians) {
        float cost = cosf(radians);
        float sint = sinf(radians);

        Mat4x4 out;

        out.m[0] = cost;
        out.m[2] = sint;
        out.m[5] = 1.0f;
        out.m[8] = -sint;
        out.m[10] = cost;
        out.m[15] = 1.0f;

        return out;
    }

    /**
     * Get a rotation matrix around the Z axis
     */
    static Mat4x4 rotationZ(float radians) {
        float cost = cosf(radians);
        float sint = sinf(radians);

        Mat4x4 out;

        out.m[0] = cost;
        out.m[1] = -sint;
        out.m[4] = sint;
        out.m[5] = cost;
        out.m[10] = 1.0f;
        out.m[15] = 1.0f;

        return out;
    }

    /**
     * Create a yaw, pitch, roll rotation matrix. The operations are applied
     * in the order: roll, then pitch, then yaw, around the origin
     */
    static Mat4x4 yawPitchRoll(float yaw, float pitch, float roll) {
        // return rotationY(yaw) * rotationX(pitch) * rotationZ(roll);

        Mat4x4 out;

        float cy = cosf(yaw);
        float sy = sinf(yaw);
        float cp = cosf(pitch);
        float sp = sinf(pitch);
        float cr = cosf(roll);
        float sr = sinf(roll);

        // Generated by mathematica
        out.m[0] = cr * cy + sp * sr * sy;
        out.m[1] = -cy * sr + cr * sp * sy;
        out.m[2] = cp * sy;
        out.m[3] = 0.0f;
        out.m[4] = cp * sr;
        out.m[5] = cp * cr;
        out.m[6] = -sp;
        out.m[7] = 0.0f;
        out.m[8] = cy * sp * sr - cr * sy;
        out.m[9] = cr * cy * sp + sr * sy;
        out.m[10] = cp * cy;
        out.m[11] = 0.0f;
        out.m[12] = 0.0f;
        out.m[13] = 0.0f;
        out.m[14] = 0.0f;
        out.m[15] = 1.0f;

        return out;
    }

    /**
     * Get a scaling matrix
     */
    static Mat4x4 scale(float x, float y, float z) {
        Mat4x4 out;

        out.m[0] = x;
        out.m[5] = y;
        out.m[10] = z;
        out.m[15] = 1.0f;

        return out;
    }

    /**
     * Get a translation matrix
     */
    static Mat4x4 translation(float x, float y, float z) {
        Mat4x4 out;

        out.m[0] = 1.0f;
        out.m[3] = x;
        out.m[5] = 1.0f;
        out.m[7] = y;
        out.m[10] = 1.0f;
        out.m[11] = z;
        out.m[15] = 1.0f;

        return out;
    }

    /**
     * Get the determinant of a matrix
     */
    static float determinant(Mat4x4 mat) {
        // Generated by mathematica
        return mat.m[0 * 4 + 3] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 2] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 3] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 0] + 
                 mat.m[0 * 4 + 1] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 0] + mat.m[0 * 4 + 2] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 0] - 
               mat.m[0 * 4 + 3] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 2] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 3] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 1] - 
               mat.m[0 * 4 + 0] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 1] - mat.m[0 * 4 + 2] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 0] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 1] + 
               mat.m[0 * 4 + 3] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 2] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 2] - mat.m[0 * 4 + 3] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 2] + 
               mat.m[0 * 4 + 0] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 2] + mat.m[0 * 4 + 1] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 2] - mat.m[0 * 4 + 0] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 2] - 
               mat.m[0 * 4 + 2] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 3] + mat.m[0 * 4 + 1] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 3] + mat.m[0 * 4 + 2] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 3] - 
               mat.m[0 * 4 + 0] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 3] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 3] + mat.m[0 * 4 + 0] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 3];
    }

    /**
     * Get the inverse of a matrix
     */
    static Mat4x4 inverse(Mat4x4 mat) {
        Mat4x4 out;

        float det = determinant(mat);

        // Generated by mathematica
        out.m[0]  = (-mat.m[1 * 4 + 3] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 1] + mat.m[1 * 4 + 2] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 1] + mat.m[1 * 4 + 3] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 2] - mat.m[1 * 4 + 1] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 2] - mat.m[1 * 4 + 2] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 3] + mat.m[1 * 4 + 1] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 3]) / det;
        out.m[1]  = ( mat.m[0 * 4 + 3] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 1] - mat.m[0 * 4 + 2] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 1] - mat.m[0 * 4 + 3] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 2] + mat.m[0 * 4 + 1] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 2] + mat.m[0 * 4 + 2] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 3] - mat.m[0 * 4 + 1] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 3]) / det;
        out.m[2]  = (-mat.m[0 * 4 + 3] * mat.m[1 * 4 + 2] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 2] * mat.m[1 * 4 + 3] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 3] * mat.m[1 * 4 + 1] * mat.m[3 * 4 + 2] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 3] * mat.m[3 * 4 + 2] - mat.m[0 * 4 + 2] * mat.m[1 * 4 + 1] * mat.m[3 * 4 + 3] + mat.m[0 * 4 + 1] * mat.m[1 * 4 + 2] * mat.m[3 * 4 + 3]) / det;
        out.m[3]  = ( mat.m[0 * 4 + 3] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 1] - mat.m[0 * 4 + 2] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 1] - mat.m[0 * 4 + 3] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 2] + mat.m[0 * 4 + 1] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 2] + mat.m[0 * 4 + 2] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 3] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 3]) / det;
        out.m[4]  = ( mat.m[1 * 4 + 3] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 0] - mat.m[1 * 4 + 2] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 0] - mat.m[1 * 4 + 3] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 2] + mat.m[1 * 4 + 0] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 2] + mat.m[1 * 4 + 2] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 3] - mat.m[1 * 4 + 0] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 3]) / det;
        out.m[5]  = (-mat.m[0 * 4 + 3] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 0] + mat.m[0 * 4 + 2] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 0] + mat.m[0 * 4 + 3] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 2] - mat.m[0 * 4 + 0] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 2] - mat.m[0 * 4 + 2] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 3] + mat.m[0 * 4 + 0] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 3]) / det;
        out.m[6]  = ( mat.m[0 * 4 + 3] * mat.m[1 * 4 + 2] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 2] * mat.m[1 * 4 + 3] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 3] * mat.m[1 * 4 + 0] * mat.m[3 * 4 + 2] + mat.m[0 * 4 + 0] * mat.m[1 * 4 + 3] * mat.m[3 * 4 + 2] + mat.m[0 * 4 + 2] * mat.m[1 * 4 + 0] * mat.m[3 * 4 + 3] - mat.m[0 * 4 + 0] * mat.m[1 * 4 + 2] * mat.m[3 * 4 + 3]) / det;
        out.m[7]  = (-mat.m[0 * 4 + 3] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 0] + mat.m[0 * 4 + 2] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 0] + mat.m[0 * 4 + 3] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 2] - mat.m[0 * 4 + 0] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 2] - mat.m[0 * 4 + 2] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 3] + mat.m[0 * 4 + 0] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 3]) / det;
        out.m[8]  = (-mat.m[1 * 4 + 3] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 0] + mat.m[1 * 4 + 1] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 0] + mat.m[1 * 4 + 3] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 1] - mat.m[1 * 4 + 0] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 1] - mat.m[1 * 4 + 1] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 3] + mat.m[1 * 4 + 0] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 3]) / det;
        out.m[9]  = ( mat.m[0 * 4 + 3] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 1] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 3] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 0] * mat.m[2 * 4 + 3] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 1] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 3] - mat.m[0 * 4 + 0] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 3]) / det;
        out.m[10] = (-mat.m[0 * 4 + 3] * mat.m[1 * 4 + 1] * mat.m[3 * 4 + 0] + mat.m[0 * 4 + 1] * mat.m[1 * 4 + 3] * mat.m[3 * 4 + 0] + mat.m[0 * 4 + 3] * mat.m[1 * 4 + 0] * mat.m[3 * 4 + 1] - mat.m[0 * 4 + 0] * mat.m[1 * 4 + 3] * mat.m[3 * 4 + 1] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 0] * mat.m[3 * 4 + 3] + mat.m[0 * 4 + 0] * mat.m[1 * 4 + 1] * mat.m[3 * 4 + 3]) / det;
        out.m[11] = ( mat.m[0 * 4 + 3] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 0] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 0] - mat.m[0 * 4 + 3] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 1] + mat.m[0 * 4 + 0] * mat.m[1 * 4 + 3] * mat.m[2 * 4 + 1] + mat.m[0 * 4 + 1] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 3] - mat.m[0 * 4 + 0] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 3]) / det;
        out.m[12] = ( mat.m[1 * 4 + 2] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 0] - mat.m[1 * 4 + 1] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 0] - mat.m[1 * 4 + 2] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 1] + mat.m[1 * 4 + 0] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 1] + mat.m[1 * 4 + 1] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 2] - mat.m[1 * 4 + 0] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 2]) / det;
        out.m[13] = (-mat.m[0 * 4 + 2] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 0] + mat.m[0 * 4 + 1] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 0] + mat.m[0 * 4 + 2] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 1] - mat.m[0 * 4 + 0] * mat.m[2 * 4 + 2] * mat.m[3 * 4 + 1] - mat.m[0 * 4 + 1] * mat.m[2 * 4 + 0] * mat.m[3 * 4 + 2] + mat.m[0 * 4 + 0] * mat.m[2 * 4 + 1] * mat.m[3 * 4 + 2]) / det;
        out.m[14] = ( mat.m[0 * 4 + 2] * mat.m[1 * 4 + 1] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 2] * mat.m[3 * 4 + 0] - mat.m[0 * 4 + 2] * mat.m[1 * 4 + 0] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 0] * mat.m[1 * 4 + 2] * mat.m[3 * 4 + 1] + mat.m[0 * 4 + 1] * mat.m[1 * 4 + 0] * mat.m[3 * 4 + 2] - mat.m[0 * 4 + 0] * mat.m[1 * 4 + 1] * mat.m[3 * 4 + 2]) / det;
        out.m[15] = (-mat.m[0 * 4 + 2] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 0] + mat.m[0 * 4 + 1] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 0] + mat.m[0 * 4 + 2] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 1] - mat.m[0 * 4 + 0] * mat.m[1 * 4 + 2] * mat.m[2 * 4 + 1] - mat.m[0 * 4 + 1] * mat.m[1 * 4 + 0] * mat.m[2 * 4 + 2] + mat.m[0 * 4 + 0] * mat.m[1 * 4 + 1] * mat.m[2 * 4 + 2]) / det;

        return out;
    }
};

/**
 * A ray with an origin and a direction
 */
struct Ray {

	Vec3 origin;     // Origin of the ray
	Vec3 direction;  // Direction of the ray

    Vec3 inv_direction;
    bool sign[3];

	/**
	 * Empty constructor
	 */
	Ray() {
	}

	/*
	 * Constructor accepts an origin and a direction
	 */
	Ray(Vec3 origin, Vec3 direction)
		: origin(origin), direction(direction)
	{
        inv_direction = Vec3(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);
        sign[0] = inv_direction.x < 0.0f;
        sign[1] = inv_direction.y < 0.0f;
        sign[2] = inv_direction.z < 0.0f;
	}

	/*
	 * Get the point a distance t from the origin in the direction
	 * 'direction'
	 */
	inline Vec3 at(float t) {
        Vec3 b = direction * t;
		return origin + b;
	}
};

/*
 * An axis-aligned bounding box
 */
struct AABB {
private:

	/*
	 * Swap the 32 bit floating point numbers at two locations
	 */
	inline void swap(float *a, float *b) {
		float temp = *a;
		*a = *b;
		*b = temp;
	}

public:

	Vec3 min;  // The minimum point of the AABB
	Vec3 max;  // The maximum point of the AABB

	/*
	 * Empty constructor does not generate a valid bounding box,
	 * only useful for array initialization
	 */
	AABB()
	{
	}

	/*
	 * Constructor accepts minimum and maximum points
	 */
	AABB(const Vec3 & Min, const Vec3 & Max) 
		: min(Min), max(Max)
	{
		join(Min);
		join(Max);
	}

	/*
	 * Add a point to the bounding box, adjusting the min and
	 * max points as necessary
	 */
	void join(const Vec3 & pt) {
		min.x = MIN2(min.x, pt.x);
		min.y = MIN2(min.y, pt.y);
		min.z = MIN2(min.z, pt.z);

		max.x = MAX2(max.x, pt.x);
		max.y = MAX2(max.y, pt.y);
		max.z = MAX2(max.z, pt.z);
	}

	/*
	 * Add the range specified by another bounding box, adjusting
	 * the min and max points as necessary
	 */
	void join(const AABB & box) {
		join(box.min);
		join(box.max);
	}

	/*
	 * Check whether the given ray intersects this AABB, setting
	 * the nearest and farthest intersection points in tmin_out
	 * and tmax_out
	 */
	bool intersects(const Ray & r, float *tmin_out, float *tmax_out) {
        // http://people.csail.mit.edu/amy/papers/box-jgt.pdf

        float tmin, tmax, tymin, tymax, tzmin, tzmax;

#define bounds(n) ((n) == 0 ? (min) : (max))

        tmin = (bounds(r.sign[0]).x - r.origin.x) * r.inv_direction.x;
        tmax = (bounds(1 - r.sign[0]).x - r.origin.x) * r.inv_direction.x;

        tymin = (bounds(r.sign[1]).y - r.origin.y) * r.inv_direction.y;
        tymax = (bounds(1 - r.sign[1]).y - r.origin.y) * r.inv_direction.y;

        if (tmin > tymax || tymin > tmax)
            return false;

        tmin = MAX2(tymin, tmin);
        tmax = MIN2(tymax, tmax);

        tzmin = (bounds(r.sign[2]).z - r.origin.z) * r.inv_direction.z;
        tzmax = (bounds(1 - r.sign[2]).z - r.origin.z) * r.inv_direction.z;

        if (tmin > tzmax || tzmin > tmax)
            return false;

        tmin = MAX2(tzmin, tmin);
        tmax = MIN2(tzmax, tmax);

        *tmin_out = tmin;
        *tmax_out = tmax;

        return true;
	}

	/*
	 * Whether this AABB contains a given point
	 */
	bool contains(const Vec3 & vec) {
		return (vec.x >= min.x && vec.x <= max.x && 
			    vec.y >= min.y && vec.y <= max.y && 
				vec.z >= min.z && vec.z <= max.z);
	}

	/*
	 * Whether this bounding box overlaps another at all
	 */
	bool intersectsBbox(const AABB & other) {
		if (other.min.x > max.x || other.max.x < min.x)
			return false;

		if (other.min.y > max.y || other.max.y < min.y)
			return false;

		if (other.min.z > max.z || other.max.z < min.z)
			return false;

		return true;
	}

	/*
	 * Get the center point of this bounding box
	 */
	Vec3 center() {
		return min + (max - min) * .05f;
	}

	/*
	 * Calculate the surface area of this bounding box
	 */
	float surfaceArea() {
		Vec3 ext = max - min;
		return 2 * (ext.x * ext.y + ext.x * ext.z + ext.y * ext.z);
	}

    /*
     * Split the bounding box
     */
    AABB split(float dist, int axis, AABB & left, AABB & right) {
        switch(axis) {
        case 0:
            left = AABB(min, Vec3(min.x + dist, max.y, max.z));
            right = AABB(Vec3(min.x + dist, min.y, min.z), max);
            break;
        case 1:
            left = AABB(min, Vec3(max.x, min.y + dist, max.z));
            right = AABB(Vec3(min.x, min.y + dist, min.z), max);
            break;
        case 2:
            left = AABB(min, Vec3(max.x, max.y, min.z + dist));
            right = AABB(Vec3(min.x, min.y, min.z + dist), max);
            break;
        }

        return AABB(); // Shouldn't happen
    }
};

/*
 * A tuple containing two 32 bit integers
 */
struct int2 {
	int x, y;

	int2(int X, int Y)
		: x(X), y(Y)
	{
	}
};

// TODO
#define randf(min, max) (min + ((float)rand() / (float)RAND_MAX) * (max - min))

void randSphere(std::vector<Vec3> & samples, int sqrtSamples);
Vec2 randCircle(float rad);
void randHemisphereCos(Vec3 norm, std::vector<Vec3> & samples, int sqrtSamples);

#endif