/*
 * Sean James
 *
 * shape.cpp
 *
 * Library for geometric math. Base Shape class for raytracing
 * and scene graph.
 *
 */

#include "shape.h"

/*
 * Whether the given ray intersects this shape. Sets the
 * distance of the collision in 'dist'
 */
bool Shape::intersects(const Ray & ray, float *dist) {
	return false;
}

/*
 * Return the normal at a given position
 */
Vec3 Shape::normalAt(const Vec3 & pos) {
	return Vec3();
}

/*
 * Get the UV coordinate at the given position
 */
Vec2 Shape::uvAt(const Vec3 & pos) {
	return Vec2();
}

/*
 * Get the axis-aligned bounding box for this shape
 */
AABB Shape::getBBox() {
	return AABB();
}

/*
 * Whether this shape's bounding box intersects the
 * given bounding box
 */
bool Shape::containedIn(const AABB & in) {
	AABB in2 = in; // TODO
	return in2.intersectsBbox(getBBox());
}

/*
 * Add all of the potential KD-tree split planes to 
 * a vector of split planes, for this shape
 */
void Shape::getSplitPoints(vector<Vec3> & pts) {
	AABB box = getBBox();

	pts.push_back(box.min);
	pts.push_back(box.max);
}

Vec3 Shape::getPosition() {
	AABB box = getBBox();

	return box.center();
}
