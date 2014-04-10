/**
 * @file polygon.cpp
 *
 * @author Sean James
 */

#include <polygon.h>

Vertex::Vertex() {
}

Vertex::Vertex(Vec3 position, Vec3 normal, Vec2 uv, Vec4 color) 
	: position(position),
	  normal(normal),
	  uv(uv),
	  color(color)
{
}

CollisionResult::CollisionResult()
	: distance(INFINITY32F),
	  polygon(NULL)
{
}

Polygon::Polygon() {
}

Polygon::Polygon(Vertex v1, Vertex v2, Vertex v3)
	: v1(v1),
	  v2(v2),
	  v3(v3)
{
	// Edges and normal
	b = v3.position - v1.position;
	c = v2.position - v1.position;
	n = c.cross(b);

	// Choose which dimension to project
	if (abs(n.x) > abs(n.y))
		k = abs(n.x) > abs(n.z) ? 0 : 2;
	else
		k = abs(n.y) > abs(n.z) ? 1 : 2;
}

bool Polygon::intersects(Ray ray, CollisionResult *result, float t_max) {
	static const int modlookup[5] = { 0, 1, 2, 0, 1 };
	int u = modlookup[k + 1];
	int v = modlookup[k + 2];

	// Distance to plane
	float t_plane = -(ray.origin - v1.position).dot(n) / ray.direction.dot(n);

	if (t_plane <= 0.0f)// TODO || (t_plane > t_max && t_max > 0.0f))
		return false;

	// Find collision
	Vec3 H;
	H.set(u, ray.origin.get(u) + t_plane * ray.direction.get(u) - v1.position.get(u));
	H.set(v, ray.origin.get(v) + t_plane * ray.direction.get(v) - v1.position.get(v));

	float denom = (b.get(u) * c.get(v) - b.get(v) * c.get(u));

	float beta = (b.get(u) * H.get(v) - b.get(v) * H.get(u)) / denom; 
	if (beta < 0.0f)
		return false;

	float gamma = (c.get(v) * H.get(u) - c.get(u) * H.get(v)) / denom;
	if (gamma < 0.0f)
		return false;

	if (beta + gamma > 1.0f)
		return false;

	result->distance = t_plane;
	result->alpha = 1.0f - beta - gamma;
	result->beta = beta;
	result->gamma = gamma;
	result->polygon = this;

	result->ray = ray;
	result->position = ray.at(t_plane);
	result->normal = interpNormal(result);
	result->uv = interpUV(result);
	result->color = interpColor(result);

	return true;
}

Vec3 Polygon::interpNormal(CollisionResult *result) {
	Vec3 an = v1.normal * result->alpha;
	Vec3 bn = v2.normal * result->beta;
	Vec3 cn = v3.normal * result->gamma;

	return an + bn + cn;
}

Vec2 Polygon::interpUV(CollisionResult *result) {
	Vec2 auv = v1.uv * result->alpha;
	Vec2 buv = v2.uv * result->beta;
	Vec2 cuv = v3.uv * result->gamma;

	return auv + buv + cuv;
}

Vec4 Polygon::interpColor(CollisionResult *result) {
	Vec4 acolor = v1.color * result->alpha;
	Vec4 bcolor = v2.color * result->beta;
	Vec4 ccolor = v3.color * result->gamma;

	return acolor + bcolor + ccolor;
}

AABB Polygon::getBBox() {
	AABB box(v1.position, v1.position);
	box.join(v2.position);
	box.join(v3.position);

	return box;
}
