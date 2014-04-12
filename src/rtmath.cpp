#include "rtmath.h"

void randSphere(std::vector<Vec3> & samples, int sqrtSamples) {
	float sampleRange = 1.0f / sqrtSamples;

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

			Vec3 n = Vec3(1, 0, 0) * r * cosf(theta) +
				Vec3(0, 1, 0) * z +
				Vec3(0, 0, 1) * r * sinf(theta);

			samples.push_back(n);
		}
	}
}

Vec2 randCircle(float rad) {
	float t = 2 * (float)M_PI * rad * randf(0.0f, 1.0f);

	float r1 = rad * randf(0.0f, 1.0f);
	float r2 = rad * randf(0.0f, 1.0f);

	float u = r1 + r2;
	float r = u;

	if (u > 1.0f)
		r = 2.0f - u;

	return Vec2(r * cosf(t), r * sinf(t));
}

void randHemisphereCos(Vec3 norm, std::vector<Vec3> & samples, int sqrtSamples) {
	float sampleRange = 1.0f / sqrtSamples;

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
			Vec3 forward = Vec3(0, 0.3f, 0.35f);
			forward.normalize();
			
			if (abs(norm.dot(forward)) == 1.0f)
				forward = Vec3(0, -1, 0);

			Vec3 right = forward.cross(norm);
			right.normalize();
			forward = norm.cross(right);
			forward.normalize();

			Vec3 n = right * r * cosf(theta) +
				norm * sqrtf(1.0f - u1) +
				forward * r * sinf(theta);

			samples.push_back(n);
		}
	}
}