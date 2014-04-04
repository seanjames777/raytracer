#include "rtmath.h"

Vec3 randSphere(const Vec3 & origin, float rad) {
	Vec3 d = Vec3(randf(-1.0f, 1.0f), randf(-1.0f, 1.0f), randf(-1.0f, 1.0f));

	d.normalize();
	d = d * rad + origin;

	return d;
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

Vec3 randHemisphere(const Vec3 & norm) {
	float max = 0.99f;
	Vec3 d = Vec3(randf(-max, max), randf(0.0f, 1.0f), randf(-max, max));
	d.normalize();

	Vec3 forward = Vec3(0, 0, 1);

	Vec3 n = norm; // TODO

	if (1.0f - abs(n.dot(forward)) < .0001f)
		forward = Vec3(0, -1, 0);

	Vec3 right = forward.cross(norm);
	forward = n.cross(right);

	Vec3 h = n * d.y + right * d.x + forward * d.z;

	return h;
}