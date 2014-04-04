/**
 * @file cube.cpp
 *
 * @author Sean James
 */

#include <cube.h>

Cube::Cube()
	: bbox(Vec3(), Vec3())
{
}

Cube::Cube(Vec3 min, Vec3 max)
	: bbox(min, max)
{
}

bool Cube::intersects(Ray ray, float *dist) {
	float t1, t2;

	if (!bbox.intersects(ray, &t1, &t2))
		return false;

	if (t1 < 0.0f && t2 < 0.0f)
		return false;

	*dist = t1;

	if (t1 < 0.0f)
		*dist = t2;

	return true;
}

bool Cube::intersects(Ray ray, CollisionResult *result) {
	float t1, t2;

	if (!bbox.intersects(ray, &t1, &t2))
		return false;

	if (t1 < 0.0f && t2 < 0.0f)
		return false;

	result->distance = t1;

	if (t1 < 0.0f)
		result->distance = t2;

	Vec3 hit = ray.origin + ray.direction * result->distance;

	result->normal = normalAt(hit);
	result->uv = uvAt(hit);
	result->shape = this;
	result->position = hit;
	result->ray = ray;

	return true;
}

Vec3 Cube::normalAt(Vec3 pos) {
	Vec3 center = bbox.center();
	Vec3 p = pos;
	Vec3 dir = p - center;

	float ax = abs(dir.x) / (bbox.max.x - bbox.min.x);
	float ay = abs(dir.y) / (bbox.max.y - bbox.min.y);
	float az = abs(dir.z) / (bbox.max.z - bbox.min.z);

	if (ax > ay && ax > az)
		return Vec3(1, 0, 0) * SIGNF(dir.x);
	else if (ay > ax && ay > az)
		return Vec3(0, 1, 0) * SIGNF(dir.y);
	else
		return Vec3(0, 0, 1) * SIGNF(dir.z);
}

Vec2 Cube::uvAt(Vec3 pos) {
	return Vec2(0, 0);
}

AABB Cube::getBBox() {
	return bbox;
}

Vec3 Cube::getMin() {
	return bbox.min;
}

Vec3 Cube::getMax() {
	return bbox.max;
}
