/*
 * Sean James
 *
 * AACube.cpp
 *
 * An axis aligned cube Shape
 *
 */

#include "aacubeshape.h"

/*
 * Empty constructor makes a zero volume origin centered
 * cube
 */
AACubeShape::AACubeShape()
	: bbox(Vec3(), Vec3())
{
}

/*
 * Constructor accepts minimum and maximum points
 */
AACubeShape::AACubeShape(const Vec3 & min, const Vec3 & max)
	: bbox(min, max)
{
}

/* 
 * Whether the given ray intersects the cube. Sets
 * intersection distance in 'dist'.
 */
bool AACubeShape::intersects(const Ray & ray, float *dist) {
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

/* 
 * Whether the given ray intersects the cube. Sets
 * intersection distance in 'dist'.
 */
bool AACubeShape::intersects(const Ray & ray, CollisionResult *result) {
	float t1, t2;

	if (!bbox.intersects(ray, &t1, &t2))
		return false;

	if (t1 < 0.0f && t2 < 0.0f)
		return false;

	result->distance = t1;

	if (t1 < 0.0f)
		result->distance = t2;

	Vec3 rd = ray.direction; // TODO
	Vec3 ro = ray.origin; // TODO
	Vec3 hit = ro + rd * result->distance;

	result->normal = normalAt(hit);
	result->uv = uvAt(hit);
	result->shape = this;
	result->position = hit;
	result->ray = ray;

	return true;
}

/*
 * Get the normal at a given point
 */
Vec3 AACubeShape::normalAt(const Vec3 & pos) {
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

/*
 * Get the UV coordinate at a given point
 */
Vec2 AACubeShape::uvAt(const Vec3 & pos) {
	// TODO
	return Vec2(0, 0);
}

/*
 * Get the axis-aligned bounding box for this cube
 */
AABB AACubeShape::getBBox() {
	return bbox;
}

/*
 * Get the minimum point
 */
Vec3 AACubeShape::getMin() {
	return bbox.min;
}

/*
 * Set the minimum point
 */
void AACubeShape::setMin(const Vec3 & Min) {
	bbox.min = Min;
}

/*
 * Get the maximum point
 */
Vec3 AACubeShape::getMax() {
	return bbox.max;
}

/*
 * Set the maximum point
 */
void AACubeShape::setMax(const Vec3 & Max) {
	bbox.max = Max;
}
