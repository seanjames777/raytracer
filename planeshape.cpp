/*
* Sean James
*
* plane.cpp
*
* Non-axis aligned plane shape
*
*/

#include "planeshape.h"

/*
 * Recalculate the plane values. Requires normal, forward, and radius to be set
 */
void PlaneShape::refresh() {
	normal.normalize();
	forward.normalize();
	right = forward.cross(normal);
	origin = normal * radius;
}

/*
 * Constructor based on normal and distance from origin. Set width and length to 0 for an
 * infinite plane.
 */
PlaneShape::PlaneShape(const Vec3 & Normal, float Radius, const Vec3 & Forward, float Width, float Length)
	: normal(Normal), radius(Radius), width(Width), length(Length), forward(Forward)
{
	refresh();
}

/*
 * Constructor based on normal and origin point. Set width and length to 0 for an
 * infinite plane.
 */
PlaneShape::PlaneShape(const Vec3 & Normal, const Vec3 & Origin, const Vec3 & Forward, float Width, float Length) 
	: normal(Normal), width(Width), length(Length), forward(Forward)
{
	normal.normalize();
	radius = normal.dot(Origin);

	refresh();
}

/*
 * Whether the given ray intersects the plane. Sets the intersection distance in
 * 'dist'.
 */
bool PlaneShape::intersects(const Ray & ray, float *dist) {
	*dist = 0.0f;

	Vec3 rd = ray.direction; // TODO
	Vec3 ro = ray.origin; // TODO

	float d = rd.dot(normal);

	if (d == 0.0f)
		return false;

	*dist = (radius - ro.dot(normal)) / d;

	if (*dist < 0)
		return false;

	if (width != 0.0f && length != 0.0f) {
		Vec3 hit  = rd * (*dist) + ray.origin;
		Vec3 diff = hit - origin;
		
		float dx = diff.dot(right);
		float dz = diff.dot(forward);

		if (abs(dx) > width / 2.0f || abs(dz) > length / 2.0f)
			return false;
	}

	return true;
}

/*
 * Whether the given ray intersects the plane. Sets the intersection distance in
 * 'dist'.
 */
bool PlaneShape::intersects(const Ray & ray, CollisionResult *result) {
	Vec3 rd = ray.direction; // TODO
	Vec3 ro = ray.origin; // TODO

	float d = rd.dot(normal);

	if (d == 0.0f)
		return false;

	result->distance = (radius - ro.dot(normal)) / d;

	if (result->distance < 0)
		return false;

	if (width != 0.0f && length != 0.0f) {
		Vec3 hit  = rd * result->distance + ray.origin;
		Vec3 diff = hit - origin;
		
		float dx = diff.dot(right);
		float dz = diff.dot(forward);

		if (abs(dx) > width / 2.0f || abs(dz) > length / 2.0f)
			return false;
	}

	result->normal = normal;

	Vec3 hit = rd * result->distance + ray.origin;
	Vec3 diff = hit - origin;

	float u = diff.dot(right);
	float v = diff.dot(forward);

	result->uv = Vec2(u, v);
	result->shape = this;
	result->position = hit;
	result->ray = ray;

	return true;
}

/*
 * Get the normal vector at a given point
 */
Vec3 PlaneShape::normalAt(const Vec3 & pos) {
	Vec3 p = pos; // TODO
	Vec3 d = p - origin;

	// TODO allow two sided
		
	/*if (d.dot(normal) < 0)
		return -normal;
	else
		return normal;*/

	return normal;
}

/*
 * Get the UV coordinates at a given point
 */
Vec2 PlaneShape::uvAt(const Vec3 & pos) {
	Vec3 p = pos; // TODO

	Vec3 diff = p - origin;

	float u = diff.dot(right);
	float v = diff.dot(forward);

	return Vec2(u, v);
}

/*
 * Get the axis-aligned bounding box for this plane
 */
AABB PlaneShape::getBBox() {
	Vec3 d = forward * (length / 2.0f) + right * (width / 2.0f);

	if (width == 0.0f && length == 0.0f)
		d = d * INFINITY;

	return AABB(origin - d, origin + d);
}

/*
 * Get the normal vector of the plane
 */
Vec3 PlaneShape::getNormal() {
	return normal;
}

/*
 * Set the normal vector of the plane
 */
void PlaneShape::setNormal(const Vec3 & Normal) {
	normal = Normal; // TODO
	refresh();
}

/*
 * Get the forward vector of the plane
 */
Vec3 PlaneShape::getForward() {
	return forward;
}

/*
 * Set the forward vector of the plane
 */
void PlaneShape::setForward(const Vec3 & Forward) {
	forward = Forward; // TODO
	refresh();
}

/* 
 * Get the right vector of the plane
 * TODO: setter?
 */
Vec3 PlaneShape::getRight() {
	return right;
}

/*
 * Get the origin of the plane
 */
Vec3 PlaneShape::getOrigin() {
	return origin;
}

/*
 * Set the origin of the plane
 */
void PlaneShape::setOrigin(const Vec3 & Origin) {
	radius = normal.dot(Origin);
	refresh();
}

/*
 * Get the distance of the plane from the (world) origin
 */
float PlaneShape::getRadius() {
	return radius;
}

/*
 * Set the distance of the plane from the (world) origin
 */
void PlaneShape::setRadius(float Radius) {
	radius = Radius;
	refresh();
}

/*
 * Get the width of the plane
 */
float PlaneShape::getWidth() {
	return width;
}

/*
 * Set the width of the plane
 */
void PlaneShape::setWidth(float Width) {
	width = Width;
}

/*
 * Get the length of the plane
 */
float PlaneShape::getLength() {
	return length;
}

/*
 * Set the length of the plane
 */
void PlaneShape::setLength(float Length) {
	length = Length;
}
