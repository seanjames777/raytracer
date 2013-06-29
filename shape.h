/*
 * Sean James
 *
 * shape.h
 *
 * Library for geometric math. Base Shape class for raytracing
 * and scene graph.
 *
 */

#ifndef _SHAPE_H
#define _SHAPE_H

#include "defs.h"
#include "rtmath.h"
#include "kdcontainable.h"

class Shape;

/*
 * Generic Shape class that acts as a base class for other shapes.
 * Any subclass that correctly implements the interface can be
 * rendered and placed into a KD-tree.
 */
class Shape : public KDContainable {
public:

	/*
	 * Whether the given ray intersects this shape. Sets the
	 * distance of the collision in 'dist'
	 */
	virtual bool intersects(const Ray & ray, float *dist) = 0;

	/*
	 * Whether the given ray intersects this shape. Sets information
	 * about the collision in <result>
	 */
	virtual bool intersects(const Ray & ray, CollisionResult *result) = 0;

	/*
	 * Return the normal at a given position
	 */
	virtual Vec3 normalAt(const Vec3 & pos) = 0;

	/*
	 * Get the UV coordinate at the given position
	 */
	virtual Vec2 uvAt(const Vec3 & pos) = 0;

	/*
	 * Get the axis-aligned bounding box for this shape
	 */
	virtual AABB getBBox() = 0;

	/*
	 * Whether this shape's bounding box intersects the
	 * given bounding box
	 */
	virtual bool containedIn(const AABB & in);

	/*
	 * Add all of the potential KD-tree split planes to 
	 * a vector of split planes, for this shape
	 */
	virtual void getSplitPoints(vector<Vec3> & pts);

	/*
	 * Get a position for this Shape
	 */
	virtual Vec3 getPosition();
};

#endif
