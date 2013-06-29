/*
 * Sean James
 *
 * aaplaneshape.h
 *
 * An axis-aligned plane shape
 */

#ifndef _AAPLANESHAPE_H
#define _AAPLANESHAPE_H

#include "shape.h"

class AAPlaneShape : public Shape {
private:

	AAPlane plane; // Internal AAPlane

public:

	/*
	 * Constructor accepts axis (X, Y, Z) = (0, 1, 2)
	 * and distance from origin
	 */
	AAPlaneShape(int direction, float radius);

	/*
	 * Whether the given ray intersects the plane. Sets the
	 * intersection distance in 'dist'.
	 */
	bool intersects(const Ray & ray, float *dist);

	/*
	 * Whether the given ray intersects the plane. Sets the
	 * intersection distance in 'dist'.
	 */
	bool intersects(const Ray & ray, CollisionResult *result);

	/*
	 * Get the normal at a given point
	 */
	Vec3 normalAt(const Vec3 & pos);

	/*
	 * Get the UV coordinates at a given point
	 */
	Vec2 uvAt(const Vec3 & pos);

	/*
	 * Get an axis-aligned bounding box for this plane
	 */
	AABB getBBox();
};

#endif
