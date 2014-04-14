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

/**
 * @brief Information about a collision
 */
struct Collision {
	/** @brief Collision distance */
	float distance;

	/** @brief Beta barycentric coordinate */
	float beta;

	/** @brief Gamma barycentric coordinate */
	float gamma;

	/** @brief Polygon */
	int polygonID;

	/**
	 * @brief Empty constructor
	 */
	Collision();
};

/**
 * @brief Additional information about a collision
 */
struct CollisionEx {
	/** @brief Position of collision */
	Vec3 position;

	/** @brief Normal at collision location, derived from polygon vertices */
	Vec3 polyNormal;

	/** @brief Interpolated normal */
	Vec3 normal;

	/** @brief Interpolated UV */
	Vec2 uv;

	/** @brief Interpolated color */
	Vec4 color;

	/** @brief Ray */
	Ray ray;
};

#pragma pack(push, 1)

struct PolygonAccel {
	float n_u; // normal.u / normal.k
	float n_v; // normal.v / normal.h
	float n_d; // constant of plane equation
	int k;     // projection axis

	// line equation AC
	float b_nu;
	float b_nv;
	float b_d;
	int pad1;

	// line equation AB
	float c_nu;
	float c_nv;
	float c_d;
	unsigned int polygonID;

	// Bounding box
	Vec3 min;
	int pad3;
	Vec3 max;
	int pad4;

	PolygonAccel(Vec3 p1, Vec3 p2, Vec3 p3, unsigned int polygonID);

	/**
	 * @brief Ray/shape intersection test
	 *
	 * @param ray    Ray to test against
	 * @param result Will be filled with information about the collision
	 *
	 * @return Whether there was a collision
	 */
	bool intersects(Ray ray, Collision *result);

	/**
	 * @brief Get the axis-aligned bounding box for this shape
	 */
	AABB getBBox();

};

#pragma pack(pop)

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

	/** @brief Normal */
	Vec3 normal;

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
	 * @brief Get more information about a collision
	 */
	void getCollisionEx(Ray ray, Collision *collision, CollisionEx *collisionEx, bool interpolate);

};

#endif
