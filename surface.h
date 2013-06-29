/*
 * Sean James
 *
 * surface.h
 *
 * An initial base class for surface definitions
 *
 */

#ifndef _SURFACE_H
#define _SURFACE_H

#include "defs.h"
#include "rtmath.h"
#include "light.h"
#include "shape.h"

/*
 * An initial base class for surface definitions
 */
class Surface {
public:

	/*
	 * Shade the surface at the given position with the given light and return the
	 * color
	 */
	virtual Color shade(Light *light, CollisionResult *result) = 0;

	/*
	 * Get the amount of reflection to add to the shading
	 */
	virtual float getReflectionAmt() = 0;

	/*
	 * Get the amount of refraction to add to the shading
	 */
	virtual float getRefractionAmt() = 0;

	/*
	 * Get the amount of transparency to add to the shading
	 */
	virtual float getTransparencyAmt() = 0;
};

#endif
