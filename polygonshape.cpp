/*
* Sean James
*
* polygon.cpp
*
* Triangle class
*
*/

#include "polygonshape.h"

/*
 * Recalculate the values for this polygon
 */
void PolygonShape::refresh() {
	Vec3 v31 = v3.position - v1.position;
	Vec3 v21 = v2.position - v1.position;

	normal = v21.cross(v31);
	area = normal.len2();

	normalizedNormal = normal;
	normalizedNormal.normalize();

	radius = v1.position.dot(normalizedNormal);
}

/*
 * Empty constructor for array allocation
 */
PolygonShape::PolygonShape() {
}

/*
 * Constructor, accepts three vertices that make up triangle in
 * clockwise order
 */
PolygonShape::PolygonShape(const Vertex & V1, const Vertex & V2, const Vertex & V3)
	: v1(V1), v2(V2), v3(V3)
{
	refresh();
}

/*
 * Test whether a point is inside the triangle. All return values
 * will be positive if the point is inside the triangle.
 */
void PolygonShape::insideOutside(const Vec3 & q, float *a, float *b, float *c) {
	Vec3 ql = q; // TODO

	*c = (v2.position - v1.position).cross(ql - v1.position).dot(normal);
	*a = (v3.position - v2.position).cross(ql - v2.position).dot(normal);
	*b = (v1.position - v3.position).cross(ql - v3.position).dot(normal);
}

/*
 * Calculate the barycentric coordinates for a point
 */
void PolygonShape::barycentric(const Vec3 & q, float *a, float *b, float *c) {
	Vec3 ql = q;

	*c = (v2.position - v1.position).cross(ql - v1.position).dot(normal) / area;
	*a = (v3.position - v2.position).cross(ql - v2.position).dot(normal) / area;
	*b = (v1.position - v3.position).cross(ql - v3.position).dot(normal) / area;
}

/*
 * Whether the given ray intersects the polygon. Sets the distance of the
 * collision in 'dist'.
 */
bool PolygonShape::intersects(const Ray & ray, float *dist) {
	Ray r = ray; // TODO

	*dist = 0.0f;

	float d = r.direction.dot(normalizedNormal);

	if (d == 0.0f)
		return false;

	*dist = (radius - r.origin.dot(normalizedNormal)) / d;

	if (*dist < 0)
		return false;

	float a, b, c;
	Vec3 q = r.at(*dist);

	insideOutside(q, &a, &b, &c);

	if (a < 0.0f || b < 0.0f || c < 0.0f)
		return false;

	return true;
}

/*
 * Whether the given ray intersects the polygon. Sets the distance of the
 * collision in 'dist'.
 */
bool PolygonShape::intersects(const Ray & ray, CollisionResult *result) {
	Ray r = ray; // TODO

	float d = r.direction.dot(normalizedNormal);

	if (d == 0.0f)
		return false;

	result->distance = (radius - r.origin.dot(normalizedNormal)) / d;

	if (result->distance < 0)
		return false;

	float a, b, c;
	Vec3 q = r.at(result->distance);

	insideOutside(q, &a, &b, &c);

	if (a < 0.0f || b < 0.0f || c < 0.0f)
		return false;

	a /= area;
	b /= area;
	c /= area;

	Vec3 an = v1.normal * a;
	Vec3 bn = v2.normal * b;
	Vec3 cn = v3.normal * c;

	result->normal = an + bn + cn;

	Vec2 auv = v1.uv * a;
	Vec2 buv = v2.uv * b;
	Vec2 cuv = v3.uv * c;

	result->uv = auv + buv + cuv;

	result->shape = this;
	result->position = r.at(result->distance);
	result->ray = ray;

	return true;
}

/*
 * Get the interpolated normal at a given point on the polygon
 */
Vec3 PolygonShape::normalAt(const Vec3 & pos) {
	float a, b, c;
	barycentric(pos, &a, &b, &c);

	Vec3 an = v1.normal * a;
	Vec3 bn = v2.normal * b;
	Vec3 cn = v3.normal * c;

	return an + bn + cn;
}

/*
 * Get the interpolated UV coordinates at a given point on the
 * polygon
 */
Vec2 PolygonShape::uvAt(const Vec3 & pos) {
	float a, b, c;
	barycentric(pos, &a, &b, &c);

	Vec2 uv(0, 0);

	uv = uv + v1.uv * a;
	uv = uv + v2.uv * b;
	uv = uv + v3.uv * c;

	return uv;
}

/*
 * Get an axis-aligned bounding box for this polygon
 */
AABB PolygonShape::getBBox() {
	AABB box(v1.position, v1.position);
	box.join(v2.position);
	box.join(v3.position);

	return box;
}

/*
 * Get a vertex of the polygon by index
 */
Vertex PolygonShape::getVertex(int idx) {
	switch(idx) {
	case 0:
		return v1;
	case 1:
		return v2;
	case 2:
		return v3;
	}

	return Vertex(Vec3(), Vec3(), Vec2()); // Shouldn't happen
}

/*
 * Set the vertices of the polygon
 */
void PolygonShape::setVertices(const Vertex & V1, const Vertex & V2, const Vertex & V3) {
	v1 = V1;
	v2 = V2;
	v3 = V3;

	refresh();
}

/*
 * Get the plane normal for this polygon
 */
Vec3 PolygonShape::getNormal() {
	return normal;
}

/*
 * Get the distance from the world origin to the polygon's plane
 */
float PolygonShape::getRadius() {
	return radius;
}

/*
 * Get the area of the polygon
 */
float PolygonShape::getArea() {
	return area;
}

