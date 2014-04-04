/**
 * @file cube.h
 *
 * @brief An axis-aligned cube shape
 *
 * @author Sean James
 */

#ifndef _CUBE_H
#define _CUBE_H

#include <shape.h>

/**
 * An axis-aligned cube shape
 */
class Cube : public Shape {
private:

	/** @brief Bounding box/internal geometry */
	AABB bbox;

public:

	/**
	 * @brief Default constructor: zero volume cube centered around origin
	 */
	Cube();

	/**
	 * @brief Constructor
	 *
	 * @param min Minimum point
	 * @param max Maximum point
	 */
	Cube(Vec3 min, Vec3 max);

	/**
	 * @brief Ray/shape intersection test
	 *
	 * @param ray  Ray to test against
	 * @param dist Will be set to the distance along ray to collision
	 *
	 * @return Whether there was a collision
	 */
	bool intersects(Ray ray, float *dist);

	/**
	 * @brief Ray/shape intersection test
	 *
	 * @param ray    Ray to test against
	 * @param result Will be filled with information about the collision
	 *
	 * @return Whether there was a collision
	 */
	bool intersects(Ray ray, CollisionResult *result);

	/**
	 * @brief Return the normal at a given position
	 */
	Vec3 normalAt(Vec3 pos);

	/**
	 * @brief Get the UV coordinate at the given position
	 */
	Vec2 uvAt(Vec3 pos);

	/**
	 * @brief Get the axis-aligned bounding box for this shape
	 */
	AABB getBBox();

	/*
	 * Get the minimum point
	 */
	Vec3 getMin();

	/*
	 * Get the maximum point
	 */
	Vec3 getMax();

};

#endif
