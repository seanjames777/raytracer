/*
 * Sean James
 *
 * rtmath.h
 *
 * Useful mathematical constructs for raytracing. Supports two
 * implementations: a standard implementation and one that
 * uses Streaming SIMD Extensions to do many operations i
 * parallel.
 *
 */

#ifndef _RTMATH_H
#define _RTMATH_H

#include "defs.h"

// Whether streaming simd extensions should be used
// #define SSE

/*
 * A 32 bit floating point vector with 2 components: X and Y
 */
struct Vec2 {

	union {

#ifdef SSE
		__m128 data;
#endif

		struct {
			float x;          // X component
			float y;          // Y component
		};

		float components[2];
	};

#ifdef SSE

	/*
	 * Constructor accepts X and Y values
	 */
	inline Vec2(float X, float Y)
		: data(_mm_set_ps(0, 0, Y, X))
	{
	}

	/*
	 * Constructor accepts raw SSE value
	 */
	inline Vec2(__m128 Data)
		: data(Data)
	{
	}

	/*
	 * Empty constructor initializes all components to 0
	 */
	inline Vec2()
		: data(_mm_setzero_ps())
	{
	}

	/*
	 * Take the dot product of this vector with another vector
	 */
	inline float dot(const Vec2 & other) {
		return _mm_cvtss_f32(_mm_dp_ps(data, other.data, 0x31));
	}

	/*
	 * Get the squared length of this vector
	 */
	inline float len2() {
		return _mm_cvtss_f32(_mm_dp_ps(data, data, 0x31));
	}

	/*
	 * Get the length of this vector
	 */
	inline float len() {
		return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(data, data, 0x31)));
	}

	/*
	 * Add two vectors component wise
	 */
	inline Vec2 operator+(const Vec2 & other) {
		return Vec2(_mm_add_ps(data, other.data));
	}

	/*
	 * Subtract a vector from another component wise
	 */
	inline Vec2 operator-(const Vec2 & other) {
		return Vec2(_mm_sub_ps(data, other.data));
	}

	/*
	 * Negate each component of a vector
	 */
	inline Vec2 operator-() {
		return Vec2(-x, -y);
	}

	/*
	 * Multiply two vectors component wise
	 */
	inline Vec2 operator*(const Vec2 & other) {
		return Vec2(_mm_mul_ps(data, other.data));
	}

	/*
	 * Divide two vectors component wise
	 */
	inline Vec2 operator/(const Vec2 & other) {
		return Vec2(_mm_div_ps(data, other.data));
	}

	/*
	 * Multiply each component of a vector by a constant
	 */
	inline Vec2 operator*(float c) {
		return Vec2(_mm_mul_ps(data, _mm_set1_ps(c)));
	}

	/*
	 * Divide each component of a vector by a constant
	 */
	inline Vec2 operator/(float c) {
		return Vec2(_mm_div_ps(data, _mm_set1_ps(c)));
	}

	/*
	 * Scale the length of the vector to be 1
	 */
	inline void normalize() {
		data = _mm_div_ps(data, _mm_sqrt_ps(_mm_dp_ps(data, data, 0x3f)));
	}

	/*
	 * Check whether the components of two vectors are equal
	 */
	inline bool operator== (const Vec2 & other) {
		return other.x == x && other.y == y;
	}

	/*
	 * Check whether the components of two vectors are almost equal within
	 * a given range
	 */
	inline bool fuzzyEquals(const Vec2 & other, float thresh) {
		float err = abs(other.x - x);
		err += abs(other.y - y);

		return err < thresh;
	}

#else

	/*
	 * Constructor accepts X and Y values
	 */
	inline Vec2(float X, float Y)
		: x(X), y(Y)
	{
	}

	/*
	 * Empty constructor initializes all components to 0
	 */
	inline Vec2()
		: x(0.0f), y(0.0f)
	{
	}

	/*
	 * Take the dot product of this vector with another vector
	 */
	inline float dot(const Vec2 & other) {
		return x * other.x + y * other.y;
	}

	/*
	 * Get the squared length of this vector
	 */
	inline float len2() {
		return x * x + y * y;
	}

	/*
	 * Get the length of this vector
	 */
	inline float len() {
		return sqrtf(x * x + y * y);
	}

	/*
	 * Add two vectors component wise
	 */
	inline Vec2 operator+(const Vec2 & other) {
		return Vec2(x + other.x, y + other.y);
	}

	/*
	 * Subtract a vector from another component wise
	 */
	inline Vec2 operator-(const Vec2 & other) {
		return Vec2(x - other.x, y - other.y);
	}

	/*
	 * Negate each component of a vector
	 */
	inline Vec2 operator-() {
		return Vec2(-x, -y);
	}

	/*
	 * Multiply two vectors component wise
	 */
	inline Vec2 operator*(const Vec2 & other) {
		return Vec2(x * other.x, y * other.y);
	}

	/*
	 * Divide two vectors component wise
	 */
	inline Vec2 operator/(const Vec2 & other) {
		return Vec2(x / other.x, y / other.y);
	}

	/*
	 * Multiply each component of a vector by a constant
	 */
	inline Vec2 operator*(float c) {
		return Vec2(x * c, y * c);
	}

	/*
	 * Divide each component of a vector by a constant
	 */
	inline Vec2 operator/(float c) {
		return Vec2(x / c, y / c);
	}

	/*
	 * Scale the length of the vector to be 1
	 */
	inline void normalize() {
		float len = sqrtf(x * x + y * y);

		x /= len;
		y /= len;
	}

	/*
	 * Check whether the components of two vectors are equal
	 */
	inline bool operator== (const Vec2 & other) {
		return x == other.x && y == other.y;
	}

	/*
	 * Check whether the components of two vectors are almost equal within
	 * a given range
	 */
	inline bool fuzzyEquals(const Vec2 & other, float thresh) {
		float err = abs(other.x - x);
		err += abs(other.y - y);

		return err < thresh;
	}

#endif

};

/*
 * A 32 bit floating point vector with 3 components: X, Y, and Z
 */
struct Vec3 {

	union {
#ifdef SSE
		__m128 data;
#endif

		struct {
			float x;          // X component
			float y;          // Y component
			float z;		  // Z component
		};

		float components[3];
	};

#ifdef SSE

	/*
	 * Constructor accepts X, Y, and Z values
	 */
	inline Vec3(float X, float Y, float Z)
		: data(_mm_set_ps(0, Z, Y, X))
	{
	}

	/*
	 * Constructor accepts raw SSE value
	 */
	inline Vec3(__m128 Data)
		: data(Data)
	{
	}

	/*
	 * Empty constructor initializes all components to 0
	 */
	inline Vec3()
		: data(_mm_setzero_ps())
	{
	}

	/*
	 * Take the dot product of this vector with another vector
	 */
	inline float dot(const Vec3 & other) {
		return _mm_cvtss_f32(_mm_dp_ps(data, other.data, 0x71));
	}

	/*
	 * Get the squared length of this vector
	 */
	inline float len2() {
		return _mm_cvtss_f32(_mm_dp_ps(data, data, 0x71));
	}

	/*
	 * Get the length of this vector
	 */
	inline float len() {
		return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(data, data, 0x71)));
	}

	/*
	 * Add two vectors component wise
	 */
	inline Vec3 operator+(const Vec3 & other) {
		return Vec3(_mm_add_ps(data, other.data));
	}

	/*
	 * Subtract a vector from another component wise
	 */
	inline Vec3 operator-(const Vec3 & other) {
		return Vec3(_mm_sub_ps(data, other.data));
	}

	/*
	 * Negate each component of a vector
	 */
	inline Vec3 operator-() {
		return Vec3(-x, -y, -z);
	}

	/*
	 * Multiply two vectors component wise
	 */
	inline Vec3 operator*(const Vec3 & other) {
		return Vec3(_mm_mul_ps(data, other.data));
	}

	/*
	 * Divide two vectors component wise
	 */
	inline Vec3 operator/(const Vec3 & other) {
		return Vec3(_mm_div_ps(data, other.data));
	}

	/*
	 * Multiply each component of a vector by a constant
	 */
	inline Vec3 operator*(float c) {
		return Vec3(_mm_mul_ps(data, _mm_set1_ps(c)));
	}

	/*
	 * Divide each component of a vector by a constant
	 */
	inline Vec3 operator/(float c) {
		return Vec3(_mm_div_ps(data, _mm_set1_ps(c)));
	}

	/*
	 * Scale the length of the vector to be 1
	 */
	inline void normalize() {
		data = _mm_div_ps(data, _mm_sqrt_ps(_mm_dp_ps(data, data, 0x7f)));
	}

	/*
	 * Check whether the components of two vectors are equal
	 */
	inline bool operator== (const Vec3 & other) {
		return other.x == x && other.y == y && other.z == z;
	}

	/*
	 * Check whether the components of two vectors are almost equal within
	 * a given range
	 */
	inline bool fuzzyEquals(const Vec3 & other, float thresh) {
		float err = abs(other.x - x);
		err += abs(other.y - y);
		err += abs(other.z - z);

		return err < thresh;
	}

	/*
	 * Reflect this vector across a normal vector
	 */
	inline Vec3 reflect(const Vec3 & across) {
		float amt = -2.0f * dot(across);
		Vec3 b = across;
		return *this + b * amt;
	}

	/*
	 * Get a refracted vector according to Snell's law
	 */
	inline Vec3 refract(const Vec3 & across, float n1, float n2) {
		float n = n1 / n2;
		float c1 = dot(across);
		float c2 = sqrtf(1.0f - n * n * (1.0f - c1 * c1));
		float b = (n * c1 - c2);
		Vec3 c = across;

		return *this * n + c * b;
	}

	/*
	 * Get the cross product of this vector and another
	 */
	inline Vec3 cross(const Vec3 & other) {
		return Vec3(_mm_sub_ps(
			_mm_mul_ps(
			_mm_shuffle_ps(data, data, _MM_SHUFFLE(3, 0, 2, 1)), 
			_mm_shuffle_ps(other.data, other.data, _MM_SHUFFLE(3, 1, 0, 2))),
			_mm_mul_ps(
			_mm_shuffle_ps(data, data, _MM_SHUFFLE(3, 1, 0, 2)), 
			_mm_shuffle_ps(other.data, other.data, _MM_SHUFFLE(3, 0, 2, 1)))));
	}

#else

	/*
	 * Constructor accepts X, Y, and Z values
	 */
	inline Vec3(float X, float Y, float Z)
		: x(X), y(Y), z(Z)
	{
	}

	/*
	 * Empty constructor initializes all components to 0
	 */
	inline Vec3()
		: x(0.0f), y(0.0f), z(0.0f)
	{
	}

	/*
	 * Take the dot product of this vector with another vector
	 */
	inline float dot(const Vec3 & other) {
		return x * other.x + y * other.y + z * other.z;
	}

	/*
	 * Get the squared length of this vector
	 */
	inline float len2() {
		return x * x + y * y + z * z;
	}

	/*
	 * Get the length of this vector
	 */
	inline float len() {
		return sqrtf(x * x + y * y + z * z);
	}

	/*
	 * Add two vectors component wise
	 */
	inline Vec3 operator+(const Vec3 & other) {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}

	/*
	 * Subtract a vector from another component wise
	 */
	inline Vec3 operator-(const Vec3 & other) {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	/*
	 * Negate each component of a vector
	 */
	inline Vec3 operator-() {
		return Vec3(-x, -y, -z);
	}

	/*
	 * Multiply two vectors component wise
	 */
	inline Vec3 operator*(const Vec3 & other) {
		return Vec3(x * other.x, y * other.y, z * other.z);
	}

	/*
	 * Divide two vectors component wise
	 */
	inline Vec3 operator/(const Vec3 & other) {
		return Vec3(x / other.x, y / other.y, z / other.z);
	}

	/*
	 * Multiply each component of a vector by a constant
	 */
	inline Vec3 operator*(float c) {
		return Vec3(x * c, y * c, z * c);
	}

	/*
	 * Divide each component of a vector by a constant
	 */
	inline Vec3 operator/(float c) {
		return Vec3(x / c, y / c, z / c);
	}

	/*
	 * Scale the length of the vector to be 1
	 */
	inline void normalize() {
		float len = sqrtf(x * x + y * y + z * z);

		x /= len;
		y /= len;
		z /= len;
	}

	/*
	 * Check whether the components of two vectors are equal
	 */
	inline bool operator== (const Vec3 & other) {
		return x == other.x && y == other.y && z == other.z;
	}

	/*
	 * Check whether the components of two vectors are almost equal within
	 * a given range
	 */
	inline bool fuzzyEquals(const Vec3 & other, float thresh) {
		float err = abs(other.x - x);
		err += abs(other.y - y);
		err += abs(other.z - z);

		return err < thresh;
	}

		/*
	 * Reflect this vector across a normal vector
	 */
	inline Vec3 reflect(const Vec3 & across) {
		float amt = -2.0f * dot(across);
		Vec3 b = across;
		return *this + b * amt;
	}

	/*
	 * Get a refracted vector according to Snell's law
	 */
	inline Vec3 refract(const Vec3 & across, float n1, float n2) {
		float n = n1 / n2;
		float c1 = dot(across);
		float c2 = sqrtf(1.0f - n * n * (1.0f - c1 * c1));
		float b = (n * c1 - c2);
		Vec3 c = across;

		return *this * n + c * b;
	}

	/*
	 * Get the cross product of this vector and another
	 */
	inline Vec3 cross(const Vec3 & other) {
		return Vec3(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x
			);
	}

#endif

};

/*
 * A 32 bit floating point vector with 4 components: X, Y, Z, and W
 */
struct Vec4 {

#ifdef SSE

	// Allow single component access into SSE value
	union {

	#ifdef SSE
			__m128 data;
	#endif

		struct {
			float x;          // X component
			float y;          // Y component
			float z;		  // Z component
			float w;          // W component
		};

		float components[4];
	};

#else

	float x;          // X component
	float y;          // Y component
	float z;          // Z component
	float w;          // W component

#endif

#ifdef SSE

	/*
	 * Constructor accepts X, Y, Z, and W values
	 */
	inline Vec4(float X, float Y, float Z, float W)
		: data(_mm_set_ps(W, Z, Y, X))
	{
	}

	/*
	 * Constructor accepts raw SSE value
	 */
	inline Vec4(__m128 Data)
		: data(Data)
	{
	}

	/*
	 * Empty constructor initializes all components to 0
	 */
	inline Vec4()
		: data(_mm_setzero_ps())
	{
	}

	/*
	 * Take the dot product of this vector with another vector
	 */
	inline float dot(const Vec4 & other) {
		return _mm_cvtss_f32(_mm_dp_ps(data, other.data, 0xf1));
	}

	/*
	 * Get the squared length of this vector
	 */
	inline float len2() {
		return _mm_cvtss_f32(_mm_dp_ps(data, data, 0xf1));
	}

	/*
	 * Get the length of this vector
	 */
	inline float len() {
		return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(data, data, 0xf1)));
	}

	/*
	 * Add two vectors component wise
	 */
	inline Vec4 operator+(const Vec4 & other) {
		return Vec4(_mm_add_ps(data, other.data));
	}

	/*
	 * Subtract a vector from another component wise
	 */
	inline Vec4 operator-(const Vec4 & other) {
		return Vec4(_mm_sub_ps(data, other.data));
	}

	/*
	 * Negate each component of a vector
	 */
	inline Vec4 operator-() {
		return Vec4(-x, -y, -z, -w);
	}

	/*
	 * Multiply two vectors component wise
	 */
	inline Vec4 operator*(const Vec4 & other) {
		return Vec4(_mm_mul_ps(data, other.data));
	}

	/*
	 * Divide two vectors component wise
	 */
	inline Vec4 operator/(const Vec4 & other) {
		return Vec4(_mm_div_ps(data, other.data));
	}

	/*
	 * Multiply each component of a vector by a constant
	 */
	inline Vec4 operator*(float c) {
		return Vec4(_mm_mul_ps(data, _mm_set1_ps(c)));
	}

	/*
	 * Divide each component of a vector by a constant
	 */
	inline Vec4 operator/(float c) {
		return Vec4(_mm_div_ps(data, _mm_set1_ps(c)));
	}

	/*
	 * Scale the length of the vector to be 1
	 */
	inline void normalize() {
		data = _mm_div_ps(data, _mm_sqrt_ps(_mm_dp_ps(data, data, 0xff)));
	}

	/*
	 * Check whether the components of two vectors are equal
	 */
	inline bool operator== (const Vec4 & other) {
		return other.x == x && other.y == y && other.z == z && other.w == w;
	}

	/*
	 * Check whether the components of two vectors are almost equal within
	 * a given range
	 */
	inline bool fuzzyEquals(const Vec4 & other, float thresh) {
		float err = abs(other.x - x);
		err += abs(other.y - y);
		err += abs(other.z - z);
		err += abs(other.w - w);

		return err < thresh;
	}

#else

	/*
	 * Constructor accepts X, Y, Z, and W values
	 */
	inline Vec4(float X, float Y, float Z, float W)
		: x(X), y(Y), z(Z), w(W)
	{
	}

	/*
	 * Empty constructor initializes all components to 0
	 */
	inline Vec4()
		: x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{
	}

	/*
	 * Take the dot product of this vector with another vector
	 */
	inline float dot(const Vec4 & other) {
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	/*
	 * Get the squared length of this vector
	 */
	inline float len2() {
		return x * x + y * y + z * z + w * w;
	}

	/*
	 * Get the length of this vector
	 */
	inline float len() {
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	/*
	 * Add two vectors component wise
	 */
	inline Vec4 operator+(const Vec4 & other) {
		return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	/*
	 * Subtract a vector from another component wise
	 */
	inline Vec4 operator-(const Vec4 & other) {
		return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	/*
	 * Negate each component of a vector
	 */
	inline Vec4 operator-() {
		return Vec4(-x, -y, -z, -w);
	}

	/*
	 * Multiply two vectors component wise
	 */
	inline Vec4 operator*(const Vec4 & other) {
		return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
	}

	/*
	 * Divide two vectors component wise
	 */
	inline Vec4 operator/(const Vec4 & other) {
		return Vec4(x / other.x, y / other.y, z / other.z, w / other.w);
	}

	/*
	 * Multiply each component of a vector by a constant
	 */
	inline Vec4 operator*(float c) {
		return Vec4(x * c, y * c, z * c, w * c);
	}

	/*
	 * Divide each component of a vector by a constant
	 */
	inline Vec4 operator/(float c) {
		return Vec4(x / c, y / c, z / c, w / c);
	}

	/*
	 * Scale the length of the vector to be 1
	 */
	inline void normalize() {
		float len = sqrtf(x * x + y * y + z * z + w * w);

		x /= len;
		y /= len;
		z /= len;
		w /= len;
	}

	/*
	 * Check whether the components of two vectors are equal
	 */
	inline bool operator== (const Vec4 & other) {
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	/*
	 * Check whether the components of two vectors are almost equal within
	 * a given range
	 */
	inline bool fuzzyEquals(const Vec4 & other, float thresh) {
		float err = abs(other.x - x);
		err += abs(other.y - y);
		err += abs(other.z - z);
		err += abs(other.w - w);

		return err < thresh;
	}

#endif

};

/*
 * A 4x4 32 bit floating point matrix
 */
struct Mat4x4 {

	float m[16];  // Elements of the matrix

	/*
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

	/*
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

	/*
	 * Transform a Vec2 by this matrix
	 */
	Vec2 operator* (Vec2 & v) {
		return Vec2(
			m[0]  * v.x + m[1]  * v.y + m[3],
			m[4]  * v.x + m[5]  * v.y + m[7]);
	}

	/*
	 * Transform a Vec3 by this matrix
	 */
	Vec3 operator* (Vec3 & v) {
		return Vec3(
			m[0]  * v.x + m[1]  * v.y + m[2]  * v.z + m[3],
			m[4]  * v.x + m[5]  * v.y + m[6]  * v.z + m[7],
			m[8]  * v.x + m[9]  * v.y + m[10] * v.z + m[11]);
	}

	/*
	 * Transform a Vec4 by this matrix
	 */
	Vec4 operator* (Vec4 & v) {
		return Vec4(
			m[0]  * v.x + m[1]  * v.y + m[2]  * v.z + m[3]  * v.w,
			m[4]  * v.x + m[5]  * v.y + m[6]  * v.z + m[7]  * v.w,
			m[8]  * v.x + m[9]  * v.y + m[10] * v.z + m[11] * v.w,
			m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w);
	}

	/*
	 * Multiply two matrices together
	 */
	Mat4x4 operator* (Mat4x4 & b) {
		Mat4x4 out;
		float r;
		int i, j, k;

		memset(out.m, 0, sizeof(float) * 16);

		for (i = 0; i < 4; i++) {
			for (k = 0; k < 4; k++) {
				r = m[i * 4 + k];

				for (j = 0; j < 4; j++)
					out.m[i * 4 + j] += r * b.m[k * 4 + j];
			}
		}
	}
};

/*
 * A ray with an origin and a direction
 */
struct Ray {

	Vec3 origin;     // Origin of the ray
	Vec3 direction;  // Direction of the ray

	/*
	 * Empty constructor
	 */
	inline Ray() {
	}

	/*
	 * Constructor accepts an origin and a direction
	 */
	inline Ray(const Vec3 & Origin, const Vec3 & Direction)
		: origin(Origin), direction(Direction)
	{
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
 * A 32 bit floating point RGBA color
 */
struct Color {

#ifdef SSE

	union {
		__m128 data;   // SSE data

		struct {
			float r;   // red channel
			float g;   // green channel
			float b;   // blue channel
			float a;   // alpha channel
		};
	};

#else

	float r;   // red channel
	float g;   // green channel
	float b;   // blue channel
	float a;   // alpha channel

#endif

	/*
	 * Constructor sets R, G, and B channels to Gray and
	 * alpha channel to 1
	 */
	Color(float Gray)
		: r(Gray), g(Gray), b(Gray), a(1.0f)
	{
	}

	/*
	 * Constructor sets R, G, and B channels and sets alpha
	 * channel to 1
	 */
	Color(float R, float G, float B)
		: r(R), g(G), b(B), a(1.0f)
	{
	}

	/*
	 * Constructor sets R, G, B, and A channels
	 */
	Color(float R, float G, float B, float A)
		: r(R), g(G), b(B), a(A)
	{
	}

	/*
	 * Constructor sets R, G, and B to 0 and alpha channel
	 * to 1
	 */
	Color()
		: r(0.0f), g(0.0f), b(0.0f), a(1.0f)
	{
	}

	/*
	 * Get the red channel as a 1 byte unsigned character
	 * between 0 and 255 inclusive
	 */
	unsigned char rInt() {
		return (unsigned char)CLAMP(0, 255, (int)(r * 255.0f));
	}

	/*
	 * Get the green channel as a 1 byte unsigned character
	 * between 0 and 255 inclusive
	 */
	unsigned char gInt() {
		return (unsigned char)CLAMP(0, 255, (int)(g * 255.0f));
	}

	/*
	 * Get the blue channel as a 1 byte unsigned character
	 * between 0 and 255 inclusive
	 */
	unsigned char bInt() {
		return (unsigned char)CLAMP(0, 255, (int)(b * 255.0f));
	}

	/*
	 * Get the alpha channel as a 1 byte unsigned character
	 * between 0 and 255 inclusive
	 */
	unsigned char aInt() {
		return (unsigned char)CLAMP(0, 255, (int)(a * 255.0f));
	}

	/*
	 * Adds the red, green, and blue channels of two colors and
	 * uses the first color's alpha channel
	 */
	Color operator+ (Color & other) {
		return Color(r + other.r, g + other.g, b + other.b, a);
	}

	/*
	 * Adds the red, green, and blue channels of another color
	 */
	Color operator+= (Color & other) {
		r += other.r;
		g += other.g;
		b += other.b;
		
		return *this;
	}

	/*
	 * Subtracts the red, green, and blue channels of two colors
	 * and uses the first color's alpha channel
	 */
	Color operator- (Color & other) {
		return Color(r - other.r, g - other.g, b - other.b, a);
	}

	/*
	 * Add each channel of two colors component wise
	 */
	Color operator* (Color & other) {
		return Color(r * other.r, g * other.g, b * other.b, a * other.a);
	}

	/*
	 * Multiply the red, green, and blue channels of this color
	 * by a constant
	 */
	Color operator *= (float c) {
		r *= c;
		g *= c;
		b *= c;

		return *this;
	}

	/*
	 * Multiply the red, green, and blue channels of a color by a constant
	 */
	Color operator* (float & c) {
		return Color(r * c, g * c, b * c, a);
	}

	/*
	 * Divide the red, green, and blue channels of a color by a constant
	 */
	Color operator/ (float & c) {
		return Color(r / c, g / c, b / c, a);
	}
};

/*
 * An axis aligned plane
 */
struct AAPlane {

	int direction;  // Direction of the plane's normal: (0, 1, 2) = (X, Y, Z)
	float radius;   // Distance from the origin to the plane

	/*
	 * Empty constructor does not generate a valid plane. Only useful for
	 * array initialization
	 */
	AAPlane() {
	}

	/*
	 * Constructor specifies plane direction and distance from origin
	 */
	AAPlane(int Direction, float Radius) 
		: direction(Direction), 
		  radius(Radius)
	{
	}

	/*
	 * Check whether the given Ray intersects the plane and return
	 * the intersection distance in t_out
	 */
	void intersects(const Ray & r, float *t_out) {
		switch(direction) {
		case 0: // X
			*t_out = (radius - r.origin.x) / r.direction.x;
			return;
		case 1: // Y
			*t_out = (radius - r.origin.y) / r.direction.y;
			return;
		case 2: // Z
			*t_out = (radius - r.origin.z) / r.direction.z;
			return;
		}
	}

	/*
	 * Get the plane's normal as a vector
	 */
	Vec3 normal() {
		switch(direction) {
		case 0: // X
			return Vec3(1, 0, 0);
		case 1: // Y
			return Vec3(0, 1, 0);
		case 2: // Z
			return Vec3(0, 0, 1);
		}

		return Vec3(); // Shouldn't happen
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
		// TODO
		float tmin, tmax;

		float txmin = (min.x - r.origin.x) / r.direction.x;
		float txmax = (max.x - r.origin.x) / r.direction.x;
		if (txmin > txmax) swap(&txmin, &txmax);

		float tymin = (min.y - r.origin.y) / r.direction.y;
		float tymax = (max.y - r.origin.y) / r.direction.y;
		if (tymin > tymax) swap(&tymin, &tymax);

		if (txmin > tymax || tymin > txmax) return false;

		tmin = MAX2(txmin, tymin);
		tmax = MIN2(txmax, tymax);

		float tzmin = (min.z - r.origin.z) / r.direction.z;
		float tzmax = (max.z - r.origin.z) / r.direction.z;
		if (tzmin > tzmax) swap(&tzmin, &tzmax);

		if (tmin > tzmax || tzmin > tmax)
			return false;

		tmin = MAX2(tmin, tzmin);
		tmax = MIN2(tmax, tzmax);

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
		Vec3 ext = max - min;
		float p5 = 0.5f;
		ext = ext * p5;
		return min + ext;
	}

	/*
	 * Calculate the surface area of this bounding box
	 */
	float surfaceArea() {
		Vec3 ext = max - min;
		return 2 * (ext.x * ext.y + ext.x * ext.z + ext.y * ext.z);
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
#define randf(min, max) ((min) + (float)rand() / ((float)RAND_MAX / ((max) - (min))))

Vec3 randSphere(const Vec3 & origin, float rad);
Vec2 randCircle(float rad);
Vec3 randHemisphere(const Vec3 & norm);

#endif