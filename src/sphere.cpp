/**
 * @file sphereshape.cpp
 *
 * @author Sean James
 */

#include "sphere.h"

Sphere::Sphere()
	: origin(Vec3(0, 0, 0)),
	  radius(0)
{
}

Sphere::Sphere(Vec3 origin, float radius)
	: origin(origin),
	  radius(radius)
{
}

bool Sphere::intersects(Ray ray, float *dist) {
	float b, disc;
	Vec3 rayOrigin = ray.origin - origin;

	b = -(rayOrigin.dot(ray.direction));

	disc = b * b - rayOrigin.len2() + radius * radius;

	if (disc < 0.0f)
		return false;

	disc = sqrtf(disc);

	float d1 = b - disc;
	float d2 = b + disc;

	*dist = MIN2(d1, d2);

	if (d1 < 0)
		*dist = d2;

	if (d2 < 0)
		return false;

	return true;
}

bool Sphere::intersects(Ray ray, CollisionResult *result) {
	float b, disc;
	Vec3 rayOrigin = ray.origin - origin;

	b = -(rayOrigin.dot(ray.direction));

	disc = b * b - rayOrigin.len2() + radius * radius;

	if (disc < 0)
		return false;

	disc = sqrtf(disc);

	float d1 = b - disc;
	float d2 = b + disc;

	result->distance = MIN2(d1, d2);

	if (d1 < 0)
		result->distance = d2;

	if (d2 < 0)
		return false;

	Vec3 hit = ray.at(result->distance);

	result->normal = normalAt(hit);
	result->uv = uvAt(hit);
	result->shape = this;
	result->position = hit;
	result->ray = ray;
	result->color = colorAt(hit);
	
	return true;
}

Vec3 Sphere::normalAt(Vec3 pos) {
	Vec3 norm = pos - origin;
	norm.normalize();

	return norm;
}

Vec2 Sphere::uvAt(Vec3 pos) {
	// TODO
	return Vec2();
}

Vec4 Sphere::colorAt(Vec3 pos) {
	// TODO
	return Vec4(1, 1, 1, 1);
}

AABB Sphere::getBBox() {
	Vec3 d(radius, radius, radius);

	return AABB(origin - d, origin + d);
}

float Sphere::getRadius() {
	return radius;
}
