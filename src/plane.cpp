/**
 * @file plane.cpp
 *
 * @author Sean James
 */

#include <plane.h>

Plane::Plane(Vec3 normal, float radius, Vec3 forward, float width, float length)
	: normal(normal),
	  radius(radius),
	  width(width),
	  length(length),
	  forward(forward)
{
	right = forward.cross(normal);
	origin = normal * radius;
}

Plane::Plane(Vec3 normal, Vec3 origin, Vec3 forward, float width, float length) 
	: normal(normal),
	  width(width),
	  length(length),
	  forward(forward)
{
	radius = normal.dot(origin);
	right = forward.cross(normal);
	origin = normal * radius;
}

bool Plane::intersects(Ray ray, float *dist) {
	*dist = 0.0f;

	float d = ray.direction.dot(normal);

	if (d == 0.0f)
		return false;

	*dist = (radius - ray.origin.dot(normal)) / d;

	if (*dist < 0)
		return false;

	if (width != 0.0f && length != 0.0f) {
		Vec3 hit  = ray.direction * (*dist) + ray.origin;
		Vec3 diff = hit - origin;
		
		float dx = diff.dot(right);
		float dz = diff.dot(forward);

		if (abs(dx) > width / 2.0f || abs(dz) > length / 2.0f)
			return false;
	}

	return true;
}

bool Plane::intersects(Ray ray, CollisionResult *result) {
	float d = ray.direction.dot(normal);

	if (d == 0.0f)
		return false;

	result->distance = (radius - ray.origin.dot(normal)) / d;

	if (result->distance < 0)
		return false;

	if (width != 0.0f && length != 0.0f) {
		Vec3 hit  = ray.direction * result->distance + ray.origin;
		Vec3 diff = hit - origin;
		
		float dx = diff.dot(right);
		float dz = diff.dot(forward);

		if (abs(dx) > width / 2.0f || abs(dz) > length / 2.0f)
			return false;
	}

	result->normal = normal;

	Vec3 hit = ray.direction * result->distance + ray.origin;
	Vec3 diff = hit - origin;

	float u = diff.dot(right);
	float v = diff.dot(forward);

	result->uv = Vec2(u, v);
	result->shape = this;
	result->position = hit;
	result->ray = ray;

	return true;
}

Vec3 Plane::normalAt(Vec3 pos) {
	// TODO: two-sided plane
	return normal;
}

Vec2 Plane::uvAt(Vec3 pos) {
	Vec3 diff = pos - origin;

	float u = diff.dot(right);
	float v = diff.dot(forward);

	return Vec2(u, v);
}

Vec4 Plane::colorAt(Vec3 pos) {
	// TODO
	return Vec4(1, 1, 1, 1);
}

AABB Plane::getBBox() {
	Vec3 d = forward * (length / 2.0f) + right * (width / 2.0f);

	if (width == 0.0f && length == 0.0f)
		d = d * INFINITY;

	return AABB(origin - d, origin + d);
}
