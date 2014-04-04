/**
 * @file surface.h
 *
 * @brief Material base class. Initially simple, only contains the parameters for shading with a
 * phong appearance.
 *
 * @author Sean James
 */

#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <defs.h>
#include <rtmath.h>
#include <light.h>
#include <shape.h>

/**
 * @brief Material base class. Initially simple, only contains the parameters for shading with a
 * phong appearance.
 */
class Material {
private:

	/** @brief Ambient light color */
	Vec3 ambient;

	/** @brief Diffuse light color */
	Vec3 diffuse;

	/** @brief Specular light color */
	Vec3 specular;

	/** @brief Specular power */
	float specularPower;

	/** @brief Reflection amount */
	float reflection;

	/** @brief Refraction amount */
	float refraction;

	/** @brief Index of refraction */	
	float ior;

public:

	/**
	 * @brief Constructor
	 *
	 * @param ambient       Ambient light color
	 * @param diffuse       Diffuse light color
	 * @param specular      Specular light color
	 * @param specularPower Specular power
	 * @param reflection    Relfection amount
	 * @param refraction    Refraction amount
	 * @param ior           Index of refraction
	 */
	Material(Vec3 ambient, Vec3 diffuse, Vec3 specular, float specularPower, float reflection,
		float refraction, float ior);

	/**
	 * @brief Shade the surface for a given collision and light source. Will be called for each
	 * light source and mixed externally with shadows, reflections, etc.
	 */
	Vec3 shade(Light *light, CollisionResult *result);

	/**
	 * @brief Get the ambient light color
	 */
	Vec3 getAmbient();

	/**
	 * @brief Get the index of refraction
	 */
	float getIndexOfRefraction();

	/**
	 * @brief Get the amount of reflection to add to the shading
	 */
	float getReflectionAmt();

	/**
	 * @brief Get the amount of refraction to add to the shading
	 */
	float getRefractionAmt();
};

#endif
