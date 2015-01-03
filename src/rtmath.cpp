#include "rtmath.h"

void randSphere(vec3 *samples, int sqrtSamples) {
    float sampleRange = 1.0f / sqrtSamples;
    int idx = 0;

    for (int i = 0; i < sqrtSamples; i++) {
        for (int j = 0; j < sqrtSamples; j++) {
            float min1 = sampleRange * i;
            float max1 = min1 + sampleRange;

            float min2 = sampleRange * j;
            float max2 = min2 + sampleRange;

            float u1 = (randf(0.0f, 1.0f) * (max1 - min1) + min1);
            float u2 = (randf(0.0f, 1.0f) * (max2 - min2) + min2);

            float theta = u2 * 2.0f * M_PI;
            float z = u1 * 2.0f - 1.0f;
            float r = sqrtf(1.0f - z * z);

            vec3 n = vec3(1, 0, 0) * r * cosf(theta) +
                vec3(0, 1, 0) * z +
                vec3(0, 0, 1) * r * sinf(theta);

            samples[idx++] = n;
        }
    }
}

vec2 randCircle(float rad) {
    float t = 2 * (float)M_PI * rad * randf(0.0f, 1.0f);

    float r1 = rad * randf(0.0f, 1.0f);
    float r2 = rad * randf(0.0f, 1.0f);

    float u = r1 + r2;
    float r = u;

    if (u > 1.0f)
        r = 2.0f - u;

    return vec2(r * cosf(t), r * sinf(t));
}

void randHemisphereCos(vec3 norm, vec3 *samples, int sqrtSamples) {
    float sampleRange = 1.0f / sqrtSamples;

    int idx = 0;

    for (int i = 0; i < sqrtSamples; i++) {
        for (int j = 0; j < sqrtSamples; j++) {
            float min1 = sampleRange * i;
            float max1 = min1 + sampleRange;

            float min2 = sampleRange * j;
            float max2 = min2 + sampleRange;

            float u1 = (randf(0.0f, 1.0f) * (max1 - min1) + min1);
            float u2 = (randf(0.0f, 1.0f) * (max2 - min2) + min2);

            float theta = u2 * 2.0f * M_PI;
            float r = sqrtf(u1);

            // TODO: find something better
            vec3 forward = normalize(vec3(0.1f, 0.3f, 0.35f));

            if (abs(dot(norm, forward)) == 1.0f)
                forward = vec3(0, -1, 0);

            vec3 right = normalize(cross(forward, norm));
            forward = normalize(cross(norm, right));

            vec3 n = right * r * cosf(theta) +
                norm * sqrtf(1.0f - u1) +
                forward * r * sinf(theta);

            samples[idx++] = n;
        }
    }
}
