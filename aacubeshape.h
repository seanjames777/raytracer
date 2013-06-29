/*
 * Sean James
 *
 * AACube.h
 *
 * An axis aligned cube Shape
 *
 */

#ifndef _AACUBE_H
#define _AACUBE_H

#include "shape.h"

/*
 * An axis aligned cube shape with variable min and max
 * points
 */
class AACubeShape : public Shape {
private:

	AABB bbox; // Internal bounding box

public:

	/*
	 * Empty constructor makes a zero volume origin centered
	 * cube
	 */
	AACubeShape();

	/*
	 * Constructor accepts minimum and maximum points
	 */
	AACubeShape(const Vec3 & min, const Vec3 & max);

	/* 
	 * Whether the given ray intersects the cube. Sets
	 * intersection distance in 'dist'.
	 */
	bool intersects(const Ray & ray, float *dist);

	/* 
	 * Whether the given ray intersects the cube. Sets
	 * intersection distance in 'dist'.
	 */
	bool intersects(const Ray & ray, CollisionResult *result);

	/*
	 * Get the normal at a given point
	 */
	Vec3 normalAt(const Vec3 & pos);

	/*
	 * Get the UV coordinate at a given point
	 */
	Vec2 uvAt(const Vec3 & pos);

	/*
	 * Get the axis-aligned bounding box for this cube
	 */
	AABB getBBox();

	/*
	 * Get the minimum point
	 */
	Vec3 getMin();

	/*
	 * Set the minimum point
	 */
	void setMin(const Vec3 & Min);

	/*
	 * Get the maximum point
	 */
	Vec3 getMax();

	/*
	 * Set the maximum point
	 */
	void setMax(const Vec3 & Max);
};

#endif
