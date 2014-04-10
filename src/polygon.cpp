/**
 * @file polygon.cpp
 *
 * @author Sean James
 */

#include <polygon.h>

// TODO Align me
static const int modlookup[5] = { 0, 1, 2, 0, 1 };

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
	Vec3 b = v3.position - v1.position;
	Vec3 c = v2.position - v1.position;
	Vec3 n = c.cross(b);

	// Choose which dimension to project
	if (abs(n.x) > abs(n.y))
		accel.k = abs(n.x) > abs(n.z) ? 0 : 2;
	else
		accel.k = abs(n.y) > abs(n.z) ? 1 : 2;

	int u = modlookup[accel.k + 1];
	int v = modlookup[accel.k + 2];

	n = n / n.get(accel.k);

	accel.n_u = n.get(u);
	accel.n_v = n.get(v);
	accel.n_d = v1.position.dot(n);

	float denom = b.get(u) * c.get(v) - b.get(v) * c.get(u);
	accel.b_nu = -b.get(v) / denom;
	accel.b_nv =  b.get(u) / denom;
	accel.b_d  =  (b.get(v) * v1.position.get(u) - b.get(u) * v1.position.get(v)) / denom;

	accel.c_nu =  c.get(v) / denom;
	accel.c_nv = -c.get(u) / denom;
	accel.c_d  =  (c.get(u) * v1.position.get(v) - c.get(v) * v1.position.get(u)) / denom;
}

bool Polygon::intersects(Ray ray, CollisionResult *result, float t_max) {
	int u = modlookup[accel.k + 1];
	int v = modlookup[accel.k + 2];

	// Find distance to plane. Start division early.
	const float nd = 1.0f / (ray.direction.get(accel.k)
		+ accel.n_u * ray.direction.get(u) + accel.n_v * ray.direction.get(v));
	const float t_plane = (accel.n_d - ray.origin.get(accel.k)
		- accel.n_u * ray.origin.get(u) - accel.n_v * ray.origin.get(v)) * nd;

	if (t_plane < 0.0f || (t_plane > t_max && t_max > 0.0f))
		return false;

	// Find collision
	const float hu = ray.origin.get(u) + t_plane * ray.direction.get(u);
	const float hv = ray.origin.get(v) + t_plane * ray.direction.get(v);

	const float beta  = (hu * accel.b_nu + hv * accel.b_nv + accel.b_d);
	if (beta < 0.0f)
		return false;

	const float gamma = (hu * accel.c_nu + hv * accel.c_nv + accel.c_d);
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
