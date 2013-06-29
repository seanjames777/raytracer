/*
 * Sean James
 *
 * aaplaneshape.cpp
 *
 * An axis-aligned plane shape
 */

#include "aaplaneshape.h"

/*
 * Constructor accepts axis (X, Y, Z) = (0, 1, 2)
 * and distance from origin
 */
AAPlaneShape::AAPlaneShape(int direction, float radius)
	: plane(direction, radius)
{
}

/*
 * Whether the given ray intersects the plane. Sets the
 * intersection distance in 'dist'.
 */
bool AAPlaneShape::intersects(const Ray & ray, float *dist) {
	plane.intersects(ray, dist);

	return *dist >= 0.0f;
}

/*
 * Whether the given ray intersects the plane. Sets the
 * intersection distance in 'dist'.
 */
bool AAPlaneShape::intersects(const Ray & ray, CollisionResult *result) {
	plane.intersects(ray, &result->distance);

	if (result->distance < 0)
		return false;

	Ray rt = ray; // TODO

	result->uv = Vec2(0, 0);
	result->normal = plane.normal();
	result->shape = this;
	result->position = rt.at(result->distance);
	result->ray = ray;

	return true;
}

/*
 * Get the normal at a given point
 */
Vec3 AAPlaneShape::normalAt(const Vec3 & pos) {
	return plane.normal();
}

/*
 * Get the UV coordinates at a given point
 */
Vec2 AAPlaneShape::uvAt(const Vec3 & pos) {
	// TODO
	return Vec2(0, 0);
}

/*
 * Get an axis-aligned bounding box for this plane
 */
AABB AAPlaneShape::getBBox() {
	switch(plane.direction) {
	case 0:
		return AABB(Vec3(-INFINITY, 0.0f, 0.0f), Vec3(INFINITY, 0.0f, 0.0f));
	case 1:
		return AABB(Vec3(0.0f, -INFINITY, 0.0f), Vec3(0.0f, INFINITY, 0.0f));
	case 2:
		return AABB(Vec3(0.0f, 0.0f, -INFINITY), Vec3(0.0f, 0.0f, INFINITY));
	}

	return AABB(); // Shouldn't happen
}
