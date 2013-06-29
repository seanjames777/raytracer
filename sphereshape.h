/*
 * Sean James
 *
 * sphere.h
 *
 * Sphere shape with an origin and a radius
 *
 */

#ifndef _SPHERE_H
#define _SPHERE_H

#include "shape.h"

/*
 * Represents a sphere shape with an origin and a radius
 */
class SphereShape : public Shape {
private:

	Vec3 origin;  // Center of the sphere
	float radius; // Radius of the sphere

public:

	/*
	 * Empty constructor sets the origin and radius to 0
	 */
	SphereShape();

	/*
	 * Constructor specifying origin and radius
	 */
	SphereShape(const Vec3 & Origin, float Radius);

	/*
	 * Whether the given ray intersects this shape. Sets the
	 * distance of the collision in 'dist'
	 */
	bool intersects(const Ray & ray, float *dist);

	/*
	 * Whether the given ray intersects this shape. Sets information
	 * about the collision in <result>
	 */
	bool intersects(const Ray & ray, CollisionResult *result);

	/*
	 * Return the normal at a given point
	 */
	Vec3 normalAt(const Vec3 & pos);

	/*
	 * Return the UV coordinates at a given point
	 */
	Vec2 uvAt(const Vec3 & pos);

	/*
	 * Get the axis-aligned bounding box of this sphere
	 */
	AABB getBBox();

	/*
	 * Get a vector to the center of the sphere
	 */
	Vec3 getOrigin();

	/*
	 * Set the center of the sphere
	 */
	void setOrigin(const Vec3 & Origin);

	/*
	 * Get the radius of the sphere
	 */
	float getRadius();

	/* 
	 * Set the radius of the sphere
	 */
	void setRadius(float Radius);
};

#endif
