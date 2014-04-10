/**
 * @file shape.h
 *
 * @brief Base class for all shapes
 *
 * @author Sean James
 */

#ifndef _SHAPE_H
#define _SHAPE_H

#include <defs.h>
#include <rtmath.h>

class Shape;

/**
 * @brief Information about a collision
 */
struct CollisionResult {
	/** @brief Distance along ray to collision */
	float distance;

	/** @brief Position of collision */
	Vec3 position;

	/** @brief Normal at collision point */
	Vec3 normal;

	/** @brief UV at collision point */
	Vec2 uv;

	/** @brief Color at collision point */
	Vec4 color;

	/** @brief Shape that was hit */
	Shape *shape;

	/** @brief Ray that was tested against */
	Ray ray;

	/**
	 * @brief Empty constructor
	 */
	CollisionResult();
};

/**
 * @brief Base class for all shapes
 */
class Shape {
public:

	/**
	 * @brief Ray/shape intersection test
	 *
	 * @param ray  Ray to test against
	 * @param dist Will be set to the distance along ray to collision
	 *
	 * @return Whether there was a collision
	 */
	virtual bool intersects(Ray ray, float *dist) = 0;

	/**
	 * @brief Ray/shape intersection test
	 *
	 * @param ray    Ray to test against
	 * @param result Will be filled with information about the collision
	 *
	 * @return Whether there was a collision
	 */
	virtual bool intersects(Ray ray, CollisionResult *result) = 0;

	/**
	 * @brief Return the normal at a given position
	 */
	virtual Vec3 normalAt(Vec3 pos) = 0;

	/**
	 * @brief Get the UV coordinate at the given position
	 */
	virtual Vec2 uvAt(Vec3 pos) = 0;

	/**
	 * @brief Get the color at the given position
	 */
	virtual Vec4 colorAt(Vec3 pos) = 0;

	/**
	 * @brief Get the axis-aligned bounding box for this shape
	 */
	virtual AABB getBBox() = 0;

	/**
	 * @brief Get the position of this shape
	 */
	Vec3 getPosition();

	/**
	 * @brief Whether this shape's bounding box intersects the given bounding box
	 */
	bool containedIn(AABB in);

};

#endif
