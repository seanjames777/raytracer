/*
 * Sean James
 *
 * kdcontainable.h
 *
 * Interface description for an object that can be placed and search for
 * in a KD-Tree
 *
 */

#ifndef _KDCONTAINABLE_H
#define _KDCONTAINABLE_H

#include "defs.h"
#include "rtmath.h"

class KDContainable;

/*
 * Information about a collision
 */
struct CollisionResult {
	float distance;
	Vec3 position;
	Vec3 normal;
	Vec2 uv;
	KDContainable *shape;
	Ray ray;
};

/*
 * Interface for an object that can be stored and searched for
 * in a KD tree
 */
class KDContainable {
public:

	/*
	 * Get whether the node intersects the given bounding box
	 */
	virtual bool containedIn(const AABB & in) = 0;

	/*
	 * Add the potential plane split positions for this node
	 * to a vector of split points. Typically the min/max of
	 * a bounding box.
	 */
	virtual void getSplitPoints(vector<Vec3> & pts) = 0;

	/*
	 * Whether the given ray intersects this node. Sets the
	 * distance of the collision in 'dist'
	 */
	virtual bool intersects(const Ray & ray, float *dist) = 0;

	virtual bool intersects(const Ray & ray, CollisionResult *result) = 0;

	/*
	 * Get a position point for this item
	 */
	virtual Vec3 getPosition() = 0;
};

#endif
