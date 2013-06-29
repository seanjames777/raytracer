/*
 * Sean James
 *
 * phongsurface.h
 *
 * A phong surface with diffuse and specular components. Also manages
 * reflection/refraction/transparency parameters and texture mapping.
 *
 */

#include "phongsurface.h"

/*
 * Default constructor accepts material parameters
 */
PhongSurface::PhongSurface(const Color & Ambient, const Color & Diffuse, const Color & Specular, float SpecularPower, float Reflection,
	float Refraction, float Transparency, Bitmap *Texture,const  Vec2 & UVScale) 
	: ambient(Ambient), 
	diffuse(Diffuse), 
	specular(Specular), 
	specularPower(SpecularPower),
	reflection(Reflection),
	refraction(Refraction),
	texture(Texture),
	uvScale(UVScale),
	transparency(Transparency)
{
}

/*
 * Shade the surface at the given position with the given light and return the
 * color
 */
Color PhongSurface::shade(Light *light, CollisionResult *result) {
	Color color =  light->getColor(result->position);
	Vec3  ldir  = -light->getDirection(result->position);

	float ndotl =  CLAMP(0.0f, 1.0f, result->normal.dot(ldir));

	Vec3  view  =  result->position - result->ray.origin;
	view.normalize();

	Vec3  ref   =  ldir.reflect(result->normal);

	float rdotv =  CLAMP(0.0f, 1.0f, ref.dot(view));
	float specf =  powf(rdotv, specularPower);
	Color spec  =  specular * specf;

	Color tex   = Color(1.0f, 1.0f, 1.0f);

	if (texture != NULL)
		tex = texture->getPixel(result->uv * uvScale);

	return color * tex * (diffuse * ndotl + ambient) + spec;
}

/*
 * Get the amount of reflection to add to the shading
 */
float PhongSurface::getReflectionAmt() {
	return reflection;
}

/*
 * Get the amount of refraction to add to the shading
 */
float PhongSurface::getRefractionAmt() {
	return refraction;
}

/*
 * Get the amount of transparency to add to the shading
 */
float PhongSurface::getTransparencyAmt() {
	return transparency;
}
