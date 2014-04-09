/**
 * @file plane.h
 *
 * @brief Possibly finite, possibly non-axis aligned plane shape
 *
 * @author Sean James
 */

#ifndef _PLANE_H
#define _PLANE_H

#include <shape.h>

/**
 * @brief Possibly finite, possibly non-axis aligned plane shape
 */
class Plane : public Shape {
private:

	/** @brief Normal */
	Vec3 normal;

	/** @brief Distance to global origin */
	float radius;

	/** @brief Origin of plane */
	Vec3 origin;

	/** @brief Forward vector of plane's coordinate system */
	Vec3 forward;

	/** @brief Right vector of plane's coordiante system */
	Vec3 right;

	/** @brief Width of the plane, as measured along right vector */
	float width;

	/** @brief Length of the plane, as measured along the forward vector */
	float length;

public:
	
	/**
	 * @brief Constructor
	 *
	 * @param normal  Normal vector
	 * @param radius  Distance to global origin
	 * @param forward Forward vector
	 * @param width   Width of the plane along right vector, or 0 for an infinite plane
	 * @param length  Length of the plane along forward vector, or 0 for an infinite plane
	 */
	Plane(Vec3 normal, float radius, Vec3 forward, float width, float length);

	/**
	 * @brief Constructor
	 *
	 * @param normal  Normal vector
	 * @param origin  Center/origin of plane
	 * @param forward Forward vector
	 * @param width   Width of the plane along right vector, or 0 for an infinite plane
	 * @param length  Length of the plane along forward vector, or 0 for an infinite plane
	 */
	Plane(Vec3 normal, Vec3 origin, Vec3 forward, float width, float length);

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

};

#endif
