/**
 * @file poly.cpp
 *
 * @author Sean James
 */

#include <poly.h>

Vertex::Vertex() {
}

Vertex::Vertex(Vec3 position, Vec3 normal, Vec2 uv) 
	: position(position),
	  normal(normal),
	  uv(uv)
{
}

Poly::Poly() {
}

Poly::Poly(Vertex v1, Vertex v2, Vertex v3)
	: v1(v1), v2(v2), v3(v3)
{
	Vec3 v31 = v3.position - v1.position;
	Vec3 v21 = v2.position - v1.position;

	normal = v21.cross(v31);
	area = normal.len2();

	normalizedNormal = normal;
	normalizedNormal.normalize();

	radius = v1.position.dot(normalizedNormal);
}

bool Poly::barycentric(Vec3 q, float *a, float *b, float *c) {
	Vec3 ql = q;

	*c = (v2.position - v1.position).cross(ql - v1.position).dot(normal) / area;
	*a = (v3.position - v2.position).cross(ql - v2.position).dot(normal) / area;
	*b = (v1.position - v3.position).cross(ql - v3.position).dot(normal) / area;

	return !(*a < 0.0f || *b < 0.0f || *c < 0.0f);
}

bool Poly::intersects(Ray ray, float *dist) {
	float d = ray.direction.dot(normalizedNormal);

	if (d == 0.0f)
		return false;

	*dist = (radius - ray.origin.dot(normalizedNormal)) / d;

	if (*dist < 0.0f)
		return false;

	float a, b, c;
	Vec3 q = ray.at(*dist);

	return barycentric(q, &a, &b, &c);
}

bool Poly::intersects(Ray ray, CollisionResult *result) {
	float d = ray.direction.dot(normalizedNormal);

	if (d >= 0.0f) // backfacing or parallel
		return false;

	result->distance = (radius - ray.origin.dot(normalizedNormal)) / d;

	if (result->distance < 0.0f)
		return false;

	float a, b, c;
	Vec3 q = ray.at(result->distance);

	if (!barycentric(q, &a, &b, &c))
		return false;

	Vec3 an = v1.normal * a;
	Vec3 bn = v2.normal * b;
	Vec3 cn = v3.normal * c;

	result->normal = an + bn + cn;

	Vec2 auv = v1.uv * a;
	Vec2 buv = v2.uv * b;
	Vec2 cuv = v3.uv * c;

	result->uv = auv + buv + cuv;

	result->shape = this;
	result->position = ray.at(result->distance);
	result->ray = ray;

	return true;
}

Vec3 Poly::normalAt(Vec3 pos) {
	float a, b, c;
	barycentric(pos, &a, &b, &c);

	Vec3 an = v1.normal * a;
	Vec3 bn = v2.normal * b;
	Vec3 cn = v3.normal * c;

	return an + bn + cn;
}

Vec2 Poly::uvAt(Vec3 pos) {
	float a, b, c;
	barycentric(pos, &a, &b, &c);

	Vec2 auv = v1.uv * a;
	Vec2 buv = v2.uv * b;
	Vec2 cuv = v3.uv * c;

	return auv + buv + cuv;
}

AABB Poly::getBBox() {
	AABB box(v1.position, v1.position);
	box.join(v2.position);
	box.join(v3.position);

	return box;
}

Vertex Poly::getVertex(int idx) {
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
