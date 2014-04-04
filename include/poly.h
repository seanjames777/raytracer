/**
 * @file poly.h
 *
 * @brief Polygon shape
 *
 * @author Sean James
 */

#ifndef _POLYGON_H
#define _POLYGON_H

#include <shape.h>

/**
 * @brief Vertex data layout
 */
struct Vertex {
public:

	/** @brief Position */
	Vec3 position;

	/** @brief Normal */
	Vec3 normal;

	/** @brief UV */
	Vec2 uv;

	/**
	 * @brief Empty constructor
	 */
	Vertex();

	/**
	 * @brief Constructor
	 *
	 * @param position Position
	 * @param normal   Normal
	 * @param uv       UV
	 */
	Vertex(Vec3 position, Vec3 normal, Vec2 uv);
};

/**
 * @brief Triangle shape
 */
class Poly : public Shape {
private:

	/** @brief Vertex 1 */
	Vertex v1;

	/** @brief Vertex 2 */
	Vertex v2;

	/** @brief Vertex 3 */
	Vertex v3;

	/** @brief Un-normalized polygon normal */
	Vec3 normal;

	/** @brief Normalized normal */
	Vec3 normalizedNormal;
	
	/** @brief Distance to polygon plane from the origin */
	float radius;

	/** @brief Polygon area */
	float area;

	/**
	 * @brief Calculate the barycentric coordinates for a point. All three output values will be
	 * non-negative if the point is inside the triangle.
	 *
	 * @param q Point to test
	 * @param a Barycentric coordinate 1
	 * @param b Barycentric coordinate 2
	 * @param c Barycentric coordinate 3
	 *
	 * @return Whether the point is inside the triangle
	 */
	bool barycentric(Vec3 q, float *a, float *b, float *c);

public:

	/**
	 * @brief Empty constructor
	 */
	Poly();

	/**
	 * @brief Constructor
	 *
	 * @param v1 Vertex 1
	 * @param v2 Vertex 2
	 * @param v3 Vertex 3
	 */
	Poly(Vertex v1, Vertex v2, Vertex v3);

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
	 * Get a vertex of the polygon by index
	 */
	Vertex getVertex(int idx);

};

#endif
