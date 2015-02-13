/**
 * @file sampling.h
 *
 * @brief Helper functions for generating random samples with various distributions and combining
 * samples with various filters.
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __SAMPLING_H
#define __SAMPLING_H

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
inline void rand1D(int count, float *samples) {
    for (int i = 0; i < count; i++)
        samples[i] = (float)rand() / (float)RAND_MAX;
}

/**
 * @brief Generate random 2D samples between 0 and 1 inclusive
 *
 * @param[in]  count   The square root of the number of samples to be generated
 * @param[out] samples 2D array of samples
 */
inline void rand2D(int count, vec2 *samples) {
    for (int i = 0; i < count * count; i++)
        samples[i] = vec2((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
}

/**
 * @brief Generate random 3D samples between 0 and 1 inclusive on the unit sphere
 *
 * @param[in]  count   The number of samples to be generated
 * @param[out] samples 3D array of samples
 */
inline void rand3D(int count, vec3 *samples) {
    for (int i = 0; i < count; i++)
        samples[i] = normalize(vec3(
            (float)rand() / (float)RAND_MAX,
            (float)rand() / (float)RAND_MAX,
            (float)rand() / (float)RAND_MAX
        ));
}

// TODO: Jitter sphere samples

/**
 * @brief Generate a random sample between 0 and 1 inclusive, jittered to reduce variance
 *
 * @param[in]  count   The number of samples to be generated, less than MAX_SAMPLES.
 * @param[out] samples 2D array of samples
 */
inline void randJittered1D(int count, float *samples) {
    float step = 1.0f / (count + 1);

    for (int i = 0; i < count; i++) {
        float u = (float)rand() / (float)RAND_MAX;
        float min = i * step;
        float max = min + step;

        samples[i] = min * (1.0f - u) + max * u;
    }
}

/**
 * @brief Generate random 2D samples between 0 and 1 inclusive, jittered to reduce variance
 *
 * @param[in]  count   The square root of the number of samples to be generated
 * @param[out] samples 2D array of samples
 */
inline void randJittered2D(int count, vec2 *samples) {
    float step = 1.0f / (count + 1);

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count; j++) {
            float u1 = (float)rand() / (float)RAND_MAX;
            float u2 = (float)rand() / (float)RAND_MAX;

            float min_u = i * step;
            float max_u = min_u + step;
            float min_v = j * step;
            float max_v = min_v + step;

            samples[i * count + j]= vec2(
                min_u * (1.0f - u1) + max_u * u1,
                min_v * (1.0f - u2) + max_v * u2
            );
        }
    }
}

/**
 * @brief Map 2D samples to a disk
 *
 * @param[in]    count   Square root of number of samples
 * @param[inout] samples Samples which will be remapped to a disk
 */
inline void mapSamplesDisk(int count, vec2 *samples) {
    for (int i = 0; i < count * count; i++) {
        float r = sqrtf(samples[i].x);
        float theta = 2.0f * (float)M_PI * samples[i].y;

        samples[i] = vec2(
            r * cosf(theta),
            r * sinf(theta)
        );
    }
}

/**
 * @brief Map 2D samples to a hemisphere, oriented along the Y axis. Sample density will follow
 * cos^{cos_pow}(theta).
 *
 * @param[in]  count       Square root of number of samples
 * @param[in]  cos_pow     Cosine power, >= 0
 * @param[in]  samples_in  Input 2D samples
 * @param[out] samples_out Output 3D normals
 */
inline void mapSamplesCosHemisphere(
    int count,
    float cos_pow,
    vec2 *samples_in,
    vec3 *samples_out)
{
    for (int i = 0; i < count * count; i++) {
        float u1 = samples_in[i].x;
        float u2 = samples_in[i].y;

        float phi = 2.0f * (float)M_PI * u1;

        float cos_phi = cosf(phi);
        float sin_phi = sinf(phi);

        float cos_theta = powf(1.0f - u2, 1.0f / (cos_pow + 1.0f));
        float sin_theta = sqrtf(1.0f - cos_theta * cos_theta);

        samples_out[i] = vec3(
            sin_theta * cos_phi,
            cos_theta,
            sin_theta * sin_phi
        );
    }
}

/**
 * @brief Map 2D samples to a hemisphere, oriented along the Y axis, with uniform density.
 *
 * @param[in]  count       Square root of number of samples
 * @param[in]  samples_in  Input 2D samples
 * @param[out] samples_out Output 3D normals
 */
inline void mapSamplesHemisphere(
    int count,
    float cos_pow,
    vec2 *samples_in,
    vec3 *samples_out)
{
    for (int i = 0; i < count * count; i++) {
        float u1 = samples_in[i].x;
        float u2 = samples_in[i].y;

        float phi = 2.0f * (float)M_PI * u1;

        float cos_phi = cosf(phi);
        float sin_phi = sinf(phi);

        float cos_theta = sqrtf(1.0f - u2);
        float sin_theta = sqrtf(1.0f - cos_theta * cos_theta);

        samples_out[i] = vec3(
            sin_theta * cos_phi,
            cos_theta,
            sin_theta * sin_phi
        );
    }
}

/**
 * @brief Utility to align vectors on a hemisphere with a normal vector
 *
 * @param[in]    count   Square root of number of samples
 * @param[inout] samples Samples to be reoriented
 * @param[in]    normal  Normal vector to align with
 */
inline void alignHemisphereNormal(int count, vec3 *samples, const vec3 & normal) {
    // TODO: Make the below makes sense, normalizes

    // Up vector is slightly offset from normal in case the normal is vertical
    vec3 v = normal;
    vec3 u = normalize(cross(normal, vec3(0.0072f, 1.0f, 0.0034f)));
    vec3 w = cross(u, v);

    for (int i = 0; i < count * count; i++)
        samples[i] = samples[i].x * u + samples[i].y * v + samples[i].z * w;
}

#endif
