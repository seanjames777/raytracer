/*
 * Sean James
 *
 * metaball.cpp
 *
 * Bad starter implementation of a metaball shape
 *
 */

#include "metaballshape.h"

/*
 * Implicit function of this metaball. Sums contributions
 * of each implicit sphere
 */
float MetaballShape::fun(Vec3 p) {
	float sum = 0.0f;

	for (int i = 0; i < npoints; i++) {
		Vec3 origin = origins[i];
		Vec3 diff = p - origin;
		float radius = radii[i];
		float radius2 = radius * radius;

		float len2 = diff.len2();

		sum += radius2 / len2;
	}

	return sum;
}

/*
 * Default constructor contains no spheres
 */
MetaballShape::MetaballShape()
	: npoints(0)
{
}

/*
 * Default constructor accepts an array of sphere origins,
 * radii, and the number of spheres
 */
MetaballShape::MetaballShape(Vec3 *Origins, float *Radii, int NPoints)
	: origins(Origins), radii(Radii), npoints(NPoints)
{
}

/*
 * Return whether this metaball intersects the given ray and
 * sets the distance of the intersection in 'dist'
 */
bool MetaballShape::intersects(Ray & ray, float *dist) {
	float delta = 0.05f;
	float t = 0.0f;

	Vec3 curr = ray.origin;

	// TODO start at the bounding box collision point
	for (int i = 0; i < 2000; i++) {
		float val = fun(curr);

		if (abs(val - 2.0f) <= 0.5f) {
			*dist = t;
			return true;
		}

		t += delta;
		curr = curr + ray.direction * delta;
	}

	return false;
}

/*
 * Get the normal at a given point
 */
Vec3 MetaballShape::normalAt(Vec3 pos) {
	Vec3 norm;

	for (int i = 0; i < npoints; i++) {
		float a = -2.0f * radii[i];
		Vec3 diff = origins[i] - pos;
		float d = diff.len2();
		a /= (d * d);

		norm = norm + diff * a;
	}

	norm.normalize();

	return norm;
}

/*
 * Get the UV coordinate at a given point
 */
Vec2 MetaballShape::uvAt(Vec3 pos) {
	// TODO
	return Vec2();
}

/*
 * Get an axis-aligned bounding box for this metaball
 */
AABB MetaballShape::getBBox() {
	float radius = radii[0];
	Vec3 origin = origins[0];

	Vec3 d(radius, radius, radius);
	AABB box(origin - d, origin + d);

	for (int i = 1; i < npoints; i++) {
		radius = radii[i];
		origin = origins[i];

		d = Vec3(radius, radius, radius);
		AABB myBox = AABB(origin - d, origin + d);

		box.join(myBox);
	}

	return box;
}

/*
 * Get a pointer to the array of sphere origins
 */
Vec3 *MetaballShape::getOrigins() {
	return origins;
}

/*
 * Get a pointer to the array of sphere radii
 */
float *MetaballShape::getRadii() {
	return radii;
}

/*
 * Get the number of spheres
 */
int MetaballShape::getNumSpheres() {
	return npoints;
}

/*
 * Set the array of spheres
 */
void MetaballShape::setSpheres(Vec3 *Origins, float *Radii, int NSpheres) {
	origins = Origins;
	radii = Radii;
	npoints = NSpheres;
}