/*
 * Sean James
 *
 * polygon.h
 *
 * Triangle class
 *
 */

#ifndef _POLYGON_H
#define _POLYGON_H

#include "shape.h"

/*
 * Stores all of the data for a single vertex
 */
struct Vertex {
	Vec3 position; // Vertex position
	Vec3 normal;   // Vertex normal
	Vec2 uv;       // Vertex UV coordinates

	/*
	 * Empty constructor for array allocation
	 */
	Vertex() {
	}

	/*
	 * Vertex constructor
	 */
	Vertex(const Vec3 & Position, const Vec3 & Normal, const Vec2 & UV) 
		: position(Position), normal(Normal), uv(UV)
	{
	}
};

/*
 * Shape made of a triplet of polygons and the space
 * between them. Based on barycentric coordinates.
 */
class PolygonShape : public Shape {
private:

	Vertex v1, v2, v3;     // 3 triangle vertices
	Vec3 normal;           // Polygon normal, not normalized 
	Vec3 normalizedNormal; // Polygon normal, normalized (TODO)
	float radius;          // Distance of polygon plane from the origin
	float area;            // Area of the polygon

	/*
	 * Recalculate the values for this polygon
	 */
	void refresh();

	/*
	 * Test whether a point is inside the triangle. All return values
	 * will be positive if the point is inside the triangle.
	 */
	void insideOutside(const Vec3 & q, float *a, float *b, float *c);

	/*
	 * Calculate the barycentric coordinates for a point
	 */
	void barycentric(const Vec3 & q, float *a, float *b, float *c);

public:

	/*
	 * Empty constructor for array allocation
	 */
	PolygonShape();

	/*
	 * Constructor, accepts three vertices that make up triangle in
	 * clockwise order
	 */
	PolygonShape(const Vertex & V1, const Vertex & V2, const Vertex & V3);

	/*
	 * Whether the given ray intersects the polygon. Sets the distance of the
	 * collision in 'dist'.
	 */
	bool intersects(const Ray & ray, float *dist);

	/*
	 * Whether the given ray intersects the polygon. Sets the distance of the
	 * collision in 'dist'.
	 */
	bool intersects(const Ray & ray, CollisionResult *result);

	/*
	 * Get the interpolated normal at a given point on the polygon
	 */
	Vec3 normalAt(const Vec3 & pos);

	/*
	 * Get the interpolated UV coordinates at a given point on the
	 * polygon
	 */
	Vec2 uvAt(const Vec3 & pos);

	/*
	 * Get an axis-aligned bounding box for this polygon
	 */
	AABB getBBox();

	/*
	 * Get a vertex of the polygon by index
	 */
	Vertex getVertex(int idx);

	/*
	 * Set the vertices of the polygon
	 */
	void setVertices(const Vertex & V1, const Vertex & V2, const Vertex & V3);

	/*
	 * Get the plane normal for this polygon
	 */
	Vec3 getNormal();

	/*
	 * Get the distance from the world origin to the polygon's plane
	 */
	float getRadius();

	/*
	 * Get the area of the polygon
	 */
	float getArea();
};

#endif
