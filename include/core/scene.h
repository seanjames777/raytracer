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
#include <shader/shader.h>
#include <vector>

// TODO: Might want to make some of the pointers not-pointers, i.e. just copy
//       lights into scene directly. Cache benefits and easier memory
//       management.

/**
 * @brief Scene structure, which tracks the polygons, lights, etc. in a scene
 */
class RT_EXPORT Scene {
private:

    /** @brief Mapping from triangle indices to shaders */
    std::vector<Shader *> shaderMap;

    /** @brief Triangles */
    std::vector<Triangle> triangles;

    /** @brief Set of lights */
    std::vector<Light *> lights;

    /** @brief Camera */
    Camera *camera;

    /** @brief Output image */
    Image<float, 3> *output;

    /** @brief Environment image */
    Image<float, 3> *environment;

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
        Image<float, 3> *output,
        Sampler         *environmentSampler,
        Image<float, 3> *environment);

    /**
     * @brief Add a light to the scene
     *
     * @param light Light to add
     */
    void addLight(Light *light);

    /**
     * @brief Add a polygon to the scene
     */
    void addPoly(Triangle triangle, Shader *shader);

    /**
     * @brief Get the shader to use for a triangle
     */
    Shader *getShader(unsigned int triangleId) const;

    /**
     * @brief Get vector of triangles
     */
    const std::vector<Triangle> & getTriangles() const;

    /**
     * @brief Get a triangle by ID
     */
    const Triangle *getTriangle(unsigned int id) const;

    /**
     * @brief Get output image
     */
    Image<float, 3> *getOutput() const;

    /**
     * @brief Get environment image
     */
    const Image<float, 3> *getEnvironment() const;

    /**
     * @brief Get environment image sampler
     */
    const Sampler *getEnvironmentSampler() const;

    /**
     * @brief Get camera
     */
    const Camera *getCamera() const;

    /**
     * @brief Get lights
     */
    const std::vector<Light *> & getLights() const;

};

inline Scene::Scene(
    Camera          *camera,
    Image<float, 3> *output,
    Sampler         *environmentSampler,
    Image<float, 3> *environment)
    : camera(camera),
      output(output),
      environmentSampler(environmentSampler),
      environment(environment)
{
}

inline void Scene::addPoly(Triangle poly, Shader *shader) {
    poly.triangle_id = triangles.size();
    triangles.push_back(poly);
    shaderMap.push_back(shader);
}

inline void Scene::addLight(Light *light) {
    lights.push_back(light);
}

inline Shader *Scene::getShader(unsigned int triangleId) const {
    return shaderMap[triangleId];
}

inline const std::vector<Triangle> & Scene::getTriangles() const {
    return triangles;
}

inline const Triangle *Scene::getTriangle(unsigned int id) const {
    return &triangles[id];
}

inline Image<float, 3> *Scene::getOutput() const {
    return output;
}

inline const Image<float, 3> *Scene::getEnvironment() const {
    return environment;
}

inline const Sampler *Scene::getEnvironmentSampler() const {
    return environmentSampler;
}

inline const Camera *Scene::getCamera() const {
    return camera;
}

inline const std::vector<Light *> & Scene::getLights() const {
    return lights;
}

#endif
