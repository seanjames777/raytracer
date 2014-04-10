/**
 * @file polygon.h
 *
 * @brief Polygon shape
 *
 * @author Sean James
 */

#ifndef _POLYGON_H
#define _POLYGON_H

#include <defs.h>
#include <rtmath.h>

/**
 * @brief Vertex data layout
 */
struct Vertex {
public:

	// TODO: padding/alignment

	/** @brief Position */
	Vec3 position;

	/** @brief Normal */
	Vec3 normal;

	/** @brief UV */
	Vec2 uv;

	/** @brief Vertex color */
	Vec4 color;

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
	 * @param color    Color
	 */
	Vertex(Vec3 position, Vec3 normal, Vec2 uv, Vec4 color);
};

class Polygon;

/**
 * @brief Information about a collision
 */
struct CollisionResult {
	/** @brief Collision distance */
	float distance;

	/** @brief Alpha barycentric coordinate */
	float alpha;

	/** @brief Beta barycentric coordinate */
	float beta;

	/** @brief Gamma barycentric coordinate */
	float gamma;

	/** @brief Polygon */
	Polygon *polygon;

	/** @brief Ray */
	Ray ray;

	/** @brief Position */
	Vec3 position;

	/** @brief Normal */
	Vec3 normal;

	/** @brief UV */
	Vec2 uv;

	/** @brief Color */
	Vec4 color;

	/**
	 * @brief Empty constructor
	 */
	CollisionResult();
};

/**
 * @brief Triangle shape
 */
struct Polygon {
public:

	/** @brief Vertex 1 */
	Vertex v1;

	/** @brief Vertex 2 */
	Vertex v2;

	/** @brief Vertex 3 */
	Vertex v3;

	/** @brief v3.position - v1.position */
	Vec3 b;

	/** @brief v2.position - v1.position */
	Vec3 c;

	/** @brief c x b / (c x b).k */
	Vec3 n;

	/** @brief Projection axis */
	int k;

	/**
	 * @brief Empty constructor
	 */
	Polygon();

	/**
	 * @brief Constructor
	 *
	 * @param v1 Vertex 1
	 * @param v2 Vertex 2
	 * @param v3 Vertex 3
	 */
	Polygon(Vertex v1, Vertex v2, Vertex v3);

	/**
	 * @brief Ray/shape intersection test
	 *
	 * @param ray    Ray to test against
	 * @param result Will be filled with information about the collision
	 *
	 * @return Whether there was a collision
	 */
	bool intersects(Ray ray, CollisionResult *result, float t_max);

	/**
	 * @brief Return the normal at a given position
	 */
	Vec3 interpNormal(CollisionResult *result);

	/**
	 * @brief Get the UV coordinate at the given position
	 */
	Vec2 interpUV(CollisionResult *result);

	/**
	 * @brief Get the color at the given position
	 */
	Vec4 interpColor(CollisionResult *result);

	/**
	 * @brief Get the axis-aligned bounding box for this shape
	 */
	AABB getBBox();

};

#endif
