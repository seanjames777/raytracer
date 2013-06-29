/*
 * Sean James
 *
 * phongsurface.cpp
 *
 * A phong surface with diffuse and specular components. Also manages
 * reflection/refraction/transparency parameters and texture mapping.
 *
 */

#ifndef _PHONGSURFACE_H
#define _PHONGSURFACE_H

#include "surface.h"
#include "bitmap.h"

/*
 * A phong surface with diffuse and specular components. Also manages
 * reflection/refraction/transparency parameters and texture mapping.
*/
class PhongSurface : public Surface {
private:

	Color ambient, diffuse, specular;
	float specularPower, reflection, refraction, transparency;
	Bitmap *texture;
	Vec2 uvScale;

public:

	/*
	 * Default constructor accepts material parameters
	 */
	PhongSurface(const Color & Ambient, const Color & Diffuse, const Color & Specular, float SpecularPower, float Reflection,
		float Refraction, float Transparency, Bitmap *Texture, const Vec2 & UVScale);

	/*
	 * Shade the surface at the given position with the given light and return the
	 * color
	 */
	Color shade(Light *light, CollisionResult *result);

	/*
	 * Get the amount of reflection to add to the shading
	 */
	float getReflectionAmt();

	/*
	 * Get the amount of refraction to add to the shading
	 */
	float getRefractionAmt();

	/*
	 * Get the amount of transparency to add to the shading
	 */
	float getTransparencyAmt();
};

#endif
