/*
 * Sean James
 *
 * plane.h
 *
 * Non-axis aligned plane shape
 *
 */

#ifndef _PLANE_H
#define _PLANE_H

#include "shape.h"

/*
 * Plane shape. Not necessarily axis aligned, but slower than AAPlane. For more
 * complex (non square or infinite) shapes, use Polygons.
 */
class PlaneShape : public Shape {
private:

	Vec3 normal;  // Normal vector
	float radius; // Distance from the origin

	Vec3 origin;  // Origin (center) of the plane
	Vec3 forward; // Forward vector perpendicular to normal and right, for texturing
	Vec3 right;   // Right vector perpendicular to normal and forward, for texturing

	float width;  // Width of the plane
	float length; // Length of the plane

	/*
	 * Recalculate the plane values. Requires normal, forward, and radius to be set
	 */
	void refresh();

public:
	
	/*
	 * Constructor based on normal and distance from origin. Set width and length to 0 for an
	 * infinite plane.
	 */
	PlaneShape(const Vec3 & Normal, float Radius, const Vec3 & Forward, float Width, float Length);

	/*
	 * Constructor based on normal and origin point. Set width and length to 0 for an
	 * infinite plane.
	 */
	PlaneShape(const Vec3 & Normal, const Vec3 & Origin, const Vec3 & Forward, float Width, float Length);

	/*
	 * Whether the given ray intersects the plane. Sets the intersection distance in
	 * 'dist'.
	 */
	bool intersects(const Ray & ray, float *dist);

	/*
	 * Whether the given ray intersects the plane. Sets the intersection distance in
	 * 'dist'.
	 */
	bool intersects(const Ray & ray, CollisionResult *result);

	/*
	 * Get the normal vector at a given point
	 */
	Vec3 normalAt(const Vec3 & pos);

	/*
	 * Get the UV coordinates at a given point
	 */
	Vec2 uvAt(const Vec3 & pos);

	/*
	 * Get the axis-aligned bounding box for this plane
	 */
	AABB getBBox();

	/*
	 * Get the normal vector of the plane
	 */
	Vec3 getNormal();

	/*
	 * Set the normal vector of the plane
	 */
	void setNormal(const Vec3 & Normal);

	/*
	 * Get the forward vector of the plane
	 */
	Vec3 getForward();

	/*
	 * Set the forward vector of the plane
	 */
	void setForward(const Vec3 & Forward);

	/* 
	 * Get the right vector of the plane
	 * TODO: setter?
	 */
	Vec3 getRight();

	/*
	 * Get the origin of the plane
	 */
	Vec3 getOrigin();

	/*
	 * Set the origin of the plane
	 */
	void setOrigin(const Vec3 & Origin);

	/*
	 * Get the distance of the plane from the (world) origin
	 */
	float getRadius();

	/*
	 * Set the distance of the plane from the (world) origin
	 */
	void setRadius(float Radius);

	/*
	 * Get the width of the plane
	 */
	float getWidth();

	/*
	 * Set the width of the plane
	 */
	void setWidth(float Width);

	/*
	 * Get the length of the plane
	 */
	float getLength();

	/*
	 * Set the length of the plane
	 */
	void setLength(float Length);
};

#endif
