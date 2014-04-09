/**
 * @file scene.h
 *
 * @brief Scene structure
 *
 * @author Sean James
 */

#ifndef _SCENE_H
#define _SCENE_H

#include <defs.h>
#include <shape.h>
#include <light.h>
#include <camera.h>
#include <bitmap.h>
#include <material.h>

/**
 * @brief Scene structure, which tracks the shapes, lights, etc. in a scene
 */
struct Scene {
public:

	/** @brief Set of shapes */
	std::vector<Shape *> shapes;

	/** @brief Set of lights */
	std::vector<Light *> lights;

	/** @brief Mapping from shapes to materials */
	std::map<Shape *, Material *> materialMap;

	/** @brief Camera */
	Camera *camera;

	/** @brief Output Bitmap */
	Bitmap *output;

	/** @brief Environment bitmap */
	Bitmap *environment;

	/**
	 * @brief Constructor
	 *
	 * @param camera Camera
	 * @param output Output image
	 */
	Scene(Camera *camera, Bitmap *output, Bitmap *environment);

	/**
	 * @brief Add a shape to the scene
	 *
	 * @param shape    Shape to add
	 * @param material Material for shape
	 */
	void addShape(Shape *shape, Material *material);

	/**
	 * @brief Add a light to the scene
	 *
	 * @param light Light to add
	 */
	void addLight(Light *light);

};

#endif
