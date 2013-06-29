/*
 * Sean James
 *
 * sphere.cpp
 *
 * Sphere shape with an origin and a radius
 *
 */

#include "sphereshape.h"

/*
 * Empty constructor sets the origin and radius to 0
 */
SphereShape::SphereShape()
	: origin(Vec3(0, 0, 0)), radius(0)
{
}

/*
 * Constructor specifying origin and radius
 */
SphereShape::SphereShape(const Vec3 & Origin, float Radius)
	: origin(Origin), radius(Radius)
{
}

/*
 * Whether a ray intersects this sphere. Sets the intersection
 * distance in 'dist'.
 */
bool SphereShape::intersects(const Ray & ray, float *dist) {
	float b, disc;
	Vec3 ro = ray.origin; // TODO
	Vec3 rayOrigin = ro - origin;

	b = -(rayOrigin.dot(ray.direction));

	disc = b * b - rayOrigin.len2() + radius * radius;

	if (disc < 0)
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

/*
 * Whether the given ray intersects this shape. Sets information
 * about the collision in <result>
 */
bool SphereShape::intersects(const Ray & ray, CollisionResult *result) {
	float b, disc;
	Vec3 ro = ray.origin; // TODO
	Vec3 rayOrigin = ro - origin;
	Ray r = ray; // TODO

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

	Vec3 hit = r.at(result->distance);
	Vec3 normal = hit - origin;
	normal.normalize();

	result->normal = normal;
	result->uv = Vec2(0, 0);
	result->shape = this;
	result->position = hit;
	result->ray = ray;

	return true;
}

/*
 * Return the normal at a given point
 */
Vec3 SphereShape::normalAt(const Vec3 & pos) {
	Vec3 p = pos; // TODO
	Vec3 norm = p - origin;
	norm.normalize();

	return norm;
}

/*
 * Return the UV coordinates at a given point
 */
Vec2 SphereShape::uvAt(const Vec3 & pos) {
	// TODO

	return Vec2();
}

/*
 * Get the axis-aligned bounding box of this sphere
 */
AABB SphereShape::getBBox() {
	Vec3 d(radius, radius, radius);

	return AABB(origin - d, origin + d);
}

/*
 * Get a vector to the center of the sphere
 */
Vec3 SphereShape::getOrigin() {
	return origin;
}

/*
 * Set the center of the sphere
 */
void SphereShape::setOrigin(const Vec3 & Origin) {
	origin = Origin;
}

/*
 * Get the radius of the sphere
 */
float SphereShape::getRadius() {
	return radius;
}

/* 
 * Set the radius of the sphere
 */
void SphereShape::setRadius(float Radius) {
	radius = Radius;
}
