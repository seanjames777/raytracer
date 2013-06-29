/*
 * Sean James
 *
 * metaball.h
 *
 * Bad starter implementation of a metaball shape
 *
 */

#ifndef _METABALL_H
#define _METABALL_H

#include "shape.h"

/*
 * Metaball class based on an array of implicit spheres
 */
class MetaballShape : public Shape {
private:

	Vec3 * origins; // Array of sphere origins
	float * radii;  // Array of sphere radii
	int npoints;    // Number of sphere

	/*
	 * Implicit function of this metaball. Sums contributions
	 * of each implicit sphere
	 */
	float fun(Vec3 p);

public:

	/*
	 * Default constructor contains no spheres
	 */
	MetaballShape();

	/*
	 * Default constructor accepts an array of sphere origins,
	 * radii, and the number of spheres
	 */
	MetaballShape(Vec3 *Origins, float *Radii, int NPoints);

	/*
	 * Return whether this metaball intersects the given ray and
	 * sets the distance of the intersection in 'dist'
	 */
	bool intersects(Ray & ray, float *dist);

	/*
	 * Get the normal at a given point
	 */
	Vec3 normalAt(Vec3 pos);

	/*
	 * Get the UV coordinate at a given point
	 */
	Vec2 uvAt(Vec3 pos);

	/*
	 * Get an axis-aligned bounding box for this metaball
	 */
	AABB getBBox();

	/*
	 * Get a pointer to the array of sphere origins
	 */
	Vec3 *getOrigins();

	/*
	 * Get a pointer to the array of sphere radii
	 */
	float *getRadii();

	/*
	 * Get the number of spheres
	 */
	int getNumSpheres();

	/*
	 * Set the array of spheres
	 */
	void setSpheres(Vec3 *Origins, float *Radii, int NSpheres);
};

#endif