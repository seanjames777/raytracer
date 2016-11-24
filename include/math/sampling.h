/**
 * @file math/sampling.h
 *
 * @brief Helper functions for generating random samples with various distributions and combining
 * samples with various filters.
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __SAMPLING_H
#define __SAMPLING_H

#include <random>

// TODO: Maybe use a random table
// TODO: Add an initialization function
// TODO: Use shiny new C++ random
// TODO: Jittered N rooks thing
// TODO: Adaptive, importance sampling
// TODO: Could use SSE for some of this
// TODO: Lots of pow/sqrt/sin/cos in the hemisphere code
// TODO: Non-uniform distributions of samples, i.e. normal/poisson

/**
 * @brief Generate a random sample between 0 and 1 inclusive
 *
 * @param[in]  count   The number of samples to be generated
 * @param[out] samples 2D array of samples
 */
inline float rand1D() {
	return (float)rand() / (float)RAND_MAX;
}

/**
 * @brief Generate random 2D samples between 0 and 1 inclusive
 *
 * @param[in]  count   The square root of the number of samples to be generated
 * @param[out] samples 2D array of samples
 */
inline float2 rand2D() {
    return float2(
        (float)rand(),
        (float)rand()
    ) / (float)RAND_MAX;
}

/**
 * @brief Generate random 3D samples between 0 and 1 inclusive
 *
 * @param[in]  count   The number of samples to be generated
 * @param[out] samples 3D array of samples
 */
inline float3 rand3D() {
    return float3(
        (float)rand(),
        (float)rand(),
        (float)rand()
    ) / (float)RAND_MAX;
}

// TODO: Jitter sphere samples

/**
 * @brief Generate a random sample between 0 and 1 inclusive, jittered to reduce variance
 *
 * @param[in]  count   The number of samples to be generated, less than MAX_SAMPLES.
 * @param[out] samples 2D array of samples
 */
inline float randJittered1D(int count, int i) {
    float u = (float)rand() / (float)RAND_MAX;
	return (i + u) / count;
}

/**
 * @brief Generate random 2D samples between 0 and 1 inclusive, jittered to reduce variance
 *
 * @param[in]  count   The square root of the number of samples to be generated
 * @param[out] samples 2D array of samples
 */
// TODO: 1 / count can be pulled out
inline float2 randJittered2D(int count, int i, int j) {
	return (float2(i, j) + float2(rand(), rand()) / (float)RAND_MAX) / count;
}

/**
 * @brief Map 2D samples to a disk
 *
 * @param[in]    count   Square root of number of samples
 * @param[inout] samples Samples which will be remapped to a disk
 */
inline float2 mapDisk(const float2 & sample) {
	float r = sqrtf(sample.x);
    float theta = 2.0f * (float)M_PI * sample.y;

    return float2(cosf(theta), sinf(theta)) * r;
}

// TODO: Map samples to volume of sphere

/**
 * @brief Map 2D samples to a hemisphere, oriented along the Y axis. Sample density will follow
 * cos^{cos_pow}(theta).
 *
 * @param[in]  count       Square root of number of samples
 * @param[in]  cos_pow     Cosine power, >= 0
 * @param[in]  samples_in  Input 2D samples
 * @param[out] samples_out Output 3D normals
 */
inline float3 mapCosHemisphere(float cos_pow, const float2 & sample) {
    // From "Raytracing from the Ground Up", pg. 129
    float u1 = sample.x;
    float u2 = sample.y;

    float phi = 2.0f * (float)M_PI * u1;

    float cos_phi = cosf(phi);
    float sin_phi = sinf(phi);

    float cos_theta = powf(u2, 1.0f / (cos_pow + 1.0f));
    float sin_theta = sqrtf(1.0f - cos_theta * cos_theta);

    return float3(
        sin_theta * cos_phi,
        cos_theta,
        sin_theta * sin_phi
    );
}

/**
 * @brief Map 2D samples to a hemisphere, oriented along the Y axis, with uniform density.
 *
 * @param[in]  count       Square root of number of samples
 * @param[in]  samples_in  Input 2D samples
 * @param[out] samples_out Output 3D normals
 */
inline float3 mapHemisphere(const float2 & sample) {
    // Same as mapSamplesCosHemisphere, but cos_pow replaced with 0
    float u1 = sample.x;
    float u2 = sample.y;

    float phi = 2.0f * (float)M_PI * u1;

    float cos_phi = cosf(phi);
    float sin_phi = sinf(phi);

    float cos_theta = u2;
    float sin_theta = sqrtf(1.0f - cos_theta * cos_theta);

    return float3(
        sin_theta * cos_phi,
        cos_theta,
        sin_theta * sin_phi
    );
}

/**
 * @brief Utility to align vectors on a hemisphere with a normal vector
 *
 * @param[in]    count   Square root of number of samples
 * @param[inout] samples Samples to be reoriented
 * @param[in]    normal  Normal vector to align with
 */
inline float3 alignHemisphere(const float3 & sample, const float3 & normal) {
    // TODO: Make the below make sense, normalize

    // Up vector is slightly offset from normal in case the normal is vertical
    float3 v = normal;
    float3 u = normalize(cross(normal, float3(0.0072f, 1.0f, 0.0034f)));
    float3 w = cross(u, v);

	// TODO: FMA
    return sample.x * u + sample.y * v + sample.z * w;
}

#endif
