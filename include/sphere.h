/**
 * @file sphere.h
 *
 * @brief Sphere shape
 *
 * @author Sean James
 */

#ifndef _SPHERE_H
#define _SPHERE_H

#include <shape.h>

/**
 * @brief Sphere shape
 */
class Sphere : public Shape {
private:

	/** @brief Center of sphere */
	Vec3 origin;

	/** @brief Radius of sphere */
	float radius;

public:

	/**
	 * @brief Default constructor: origin and radius will be 0
	 */
	Sphere();

	/**
	 * @brief Constructor
	 *
	 * @param origin Center of sphere
	 * @param radius Radius of sphere
	 */
	Sphere(Vec3 origin, float radius);

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

	/**
	 * @brief Get the position of this shape
	 */
	Vec3 getPosition();

	/**
	 * @brief Get the radius of the sphere
	 */
	float getRadius();
};

#endif
