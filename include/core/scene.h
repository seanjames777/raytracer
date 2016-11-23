/**
 * @file core/scene.h
 *
 * @brief Scene class
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __SCENE_H
#define __SCENE_H

#include <core/camera.h>
#include <core/triangle.h>
#include <image/image.h>
#include <image/sampler.h>
#include <light/light.h>
#include <core/material.h>
#include <util/vector.h>
#include <vector>

// TODO: Might want to make some of the pointers not-pointers, i.e. just copy
//       lights into scene directly. Cache benefits and easier memory
//       management.

/**
 * @brief Scene structure, which tracks the polygons, lights, etc. in a scene
 */
class RT_EXPORT Scene {
private:

    /** @brief Materials */
    std::vector<Material *> materials;

    /** @brief Triangles */
    util::vector<Triangle, 16> triangles;

    /** @brief Lights */
    std::vector<Light *> lights;

    /** @brief Camera */
    Camera *camera;

    /** @brief Output image */
    Image<float, 4> *output;

    /** @brief Environment image */
    Image<float, 4> *environment;

    /** @brief Environment image sampler */
    Sampler *environmentSampler;

public:

    /**
     * @brief Constructor
     *
     * @param camera Camera
     * @param output Output image
     */
    Scene(
        Camera          *camera,
        Image<float, 4> *output,
        Sampler         *environmentSampler,
        Image<float, 4> *environment);

    /**
     * @brief Add a light to the scene
     *
     * @param light Light to add
     */
    void addLight(Light *light);

    /**
     * @brief Add a polygon to the scene
     */
    void addPoly(Triangle triangle);

	/**
	 * @brief Add a material to the scene
	 */
	void addMaterial(Material *material);

    /**
     * @brief Get a material by ID
     */
    Material *getMaterial(unsigned int id) const;

	unsigned int getNumMaterials() const;

    /**
     * @brief Get vector of triangles
     */
    const util::vector<Triangle, 16> & getTriangles() const;

    /**
     * @brief Get a triangle by ID
     */
    const Triangle *getTriangle(unsigned int id) const;

    /**
     * @brief Get output image
     */
    Image<float, 4> *getOutput() const;

    /**
     * @brief Get environment image
     */
    const Image<float, 4> *getEnvironment() const;

    /**
     * @brief Get environment image sampler
     */
    const Sampler *getEnvironmentSampler() const;

    /**
     * @brief Get camera
     */
    const Camera *getCamera() const;

	unsigned int getNumLights() const;

    /**
     * @brief Get lights
     */
    const Light *getLight(unsigned int light) const;

};

inline Scene::Scene(
    Camera          *camera,
    Image<float, 4> *output,
    Sampler         *environmentSampler,
    Image<float, 4> *environment)
    : camera(camera),
      output(output),
      environmentSampler(environmentSampler),
      environment(environment),
	  triangles(16) // TODO: might not be necessary because alignof(Triangle) should be 16
{
}

inline void Scene::addPoly(Triangle poly) {
    poly.triangle_id = triangles.size();
    triangles.push_back(poly);
}

inline void Scene::addLight(Light *light) {
    lights.push_back(light);
}

inline void Scene::addMaterial(Material *material) {
	materials.push_back(material);
}

inline Material *Scene::getMaterial(unsigned int id) const {
    return materials[id];
}

inline unsigned int Scene::getNumMaterials() const {
	return materials.size();
}

inline const util::vector<Triangle, 16> & Scene::getTriangles() const {
    return triangles;
}

inline const Triangle *Scene::getTriangle(unsigned int id) const {
    return &triangles[id];
}

inline Image<float, 4> *Scene::getOutput() const {
    return output;
}

inline const Image<float, 4> *Scene::getEnvironment() const {
    return environment;
}

inline const Sampler *Scene::getEnvironmentSampler() const {
    return environmentSampler;
}

inline const Camera *Scene::getCamera() const {
    return camera;
}

inline unsigned int Scene::getNumLights() const {
	return lights.size();
}

inline const Light *Scene::getLight(unsigned int light) const {
    return lights[light];
}

#endif
