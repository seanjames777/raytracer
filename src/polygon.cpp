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

Collision::Collision()
	: distance(INFINITY32F)
{
}

PolygonAccel::PolygonAccel(Vec3 v1, Vec3 v2, Vec3 v3, unsigned int polygonID)
	: polygonID(polygonID)
{
	// Edges and normal
	Vec3 b = v3 - v1;
	Vec3 c = v2 - v1;
	Vec3 n = c.cross(b);

	// Choose which dimension to project
	if (abs(n.x) > abs(n.y))
		k = abs(n.x) > abs(n.z) ? 0 : 2;
	else
		k = abs(n.y) > abs(n.z) ? 1 : 2;

	int u = modlookup[k + 1];
	int v = modlookup[k + 2];

	n = n / n.get(k);

	n_u = n.get(u);
	n_v = n.get(v);
	n_d = v1.dot(n);

	float denom = b.get(u) * c.get(v) - b.get(v) * c.get(u);
	b_nu = -b.get(v) / denom;
	b_nv =  b.get(u) / denom;
	b_d  =  (b.get(v) * v1.get(u) - b.get(u) * v1.get(v)) / denom;

	c_nu =  c.get(v) / denom;
	c_nv = -c.get(u) / denom;
	c_d  =  (c.get(u) * v1.get(v) - c.get(v) * v1.get(u)) / denom;

	AABB box(v1, v1);
	box.join(v2);
	box.join(v3);

	min = box.min;
	max = box.max;
}

AABB PolygonAccel::getBBox() {
	return AABB(min, max);
}

bool PolygonAccel::intersects(Ray ray, Collision *result) {
	int u = modlookup[k + 1];
	int v = modlookup[k + 2];

	// Find distance to plane. Start division early.
	const float nd = 1.0f / (ray.direction.get(k)
		+ n_u * ray.direction.get(u) + n_v * ray.direction.get(v));
	const float t_plane = (n_d - ray.origin.get(k)
		- n_u * ray.origin.get(u) - n_v * ray.origin.get(v)) * nd;

	// Backfacing or parallel to ray
	if (t_plane <= 0.0f)
		return false;

	// Find collision
	const float hu = ray.origin.get(u) + t_plane * ray.direction.get(u);
	const float hv = ray.origin.get(v) + t_plane * ray.direction.get(v);

	const float beta  = (hu * b_nu + hv * b_nv + b_d);
	if (beta < 0.0f)
		return false;

	const float gamma = (hu * c_nu + hv * c_nv + c_d);
	if (gamma < 0.0f)
		return false;

	if (beta + gamma > 1.0f)
		return false;

	result->distance = t_plane;
	result->beta = beta;
	result->gamma = gamma;
	result->polygonID = polygonID;

	return true;
}

Polygon::Polygon() {
}

Polygon::Polygon(Vertex v1, Vertex v2, Vertex v3)
	: v1(v1),
	  v2(v2),
	  v3(v3)
{
	Vec3 b = v3.position - v1.position;
	Vec3 c = v2.position - v1.position;

	b.normalize();
	c.normalize();

	normal = c.cross(b);
	normal.normalize();
}

void Polygon::getCollisionEx(Ray ray, Collision *collision, CollisionEx *collisionEx,
	bool interpolate)
{
	collisionEx->ray = ray;
	collisionEx->position = ray.at(collision->distance);
	collisionEx->polyNormal = normal;

	if (interpolate) {
		float alpha = 1.0f - collision->beta - collision->gamma;

		Vec3 an = v1.normal * alpha;
		Vec3 bn = v2.normal * collision->beta;
		Vec3 cn = v3.normal * collision->gamma;
		collisionEx->normal = an + bn + cn;

		Vec2 auv = v1.uv * alpha;
		Vec2 buv = v2.uv * collision->beta;
		Vec2 cuv = v3.uv * collision->gamma;
		collisionEx->uv = auv + buv + cuv;

		Vec4 acolor = v1.color * alpha;
		Vec4 bcolor = v2.color * collision->beta;
		Vec4 ccolor = v3.color * collision->gamma;
		collisionEx->color = acolor + bcolor + ccolor;
	}
}
