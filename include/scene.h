/**
 * @file scene.h
 *
 * @brief Scene structure
 *
 * @author Sean James
 */

#ifndef _SCENE_H
#define _SCENE_H

#include <polygon.h>
#include <light.h>
#include <camera.h>
#include <image.h>
#include <material.h>
#include <vector>

/**
 * @brief Scene structure, which tracks the polygons, lights, etc. in a scene
 */
class Scene {
public:

    unsigned int poly_id;

    std::vector<Triangle> triangles;

    /** @brief Set of lights */
    std::vector<Light *> lights;

    /** @brief Mapping from polygons to materials */
    std::vector<Material *> materialMap;

    /** @brief Camera */
    Camera *camera;

    /** @brief Output image */
    std::shared_ptr<Image> output;

    /** @brief Environment image */
    std::shared_ptr<Image> environment;

    std::shared_ptr<Sampler> environment_sampler;

    /**
     * @brief Constructor
     *
     * @param camera Camera
     * @param output Output image
     */
    Scene(Camera *camera, std::shared_ptr<Image> output,
        std::shared_ptr<Sampler> environment_sampler,
        std::shared_ptr<Image> environment);

    /**
     * @brief Add a light to the scene
     *
     * @param light Light to add
     */
    void addLight(Light *light);

    void addPoly(Triangle triangle, Material *material);

};

#endif
