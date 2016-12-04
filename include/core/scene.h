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
#include <util/meshloader.h>
#include <util/path.h>
#include <vector>

// TODO: Might want to make some of the pointers not-pointers, i.e. just copy
//       lights into scene directly. Cache benefits and easier memory
//       management.

/**
 * @brief Scene structure, which tracks the polygons, lights, etc. in a scene
 */
class RT_EXPORT Scene {
private:

    std::vector<Material *>     materials;
    util::vector<Triangle, 16>  triangles;
    std::vector<Light *>        lights;
    Camera                     *camera;
    Image<float, 4>            *environment;
    Sampler                    *environmentSampler;

public:

    Scene();

    virtual ~Scene();

    void addLight(Light *light);

    void addMesh(Mesh *mesh,
        const float3 & translation,
        const float3 & rotation,
        float          scale);

    void setCamera(Camera *camera);

    void setEnvironment(Image<float, 4> *environment);

    void setEnvironmentSampler(Sampler *sampler);

    Image<float, 4> *addTexture(std::string name);

    Material *getMaterial(unsigned int id) const;

    unsigned int getNumMaterials() const;

    util::vector<Triangle, 16> & getTriangles();

    const Triangle *getTriangle(unsigned int id) const;

    const Image<float, 4> *getEnvironment() const;

    const Sampler *getEnvironmentSampler() const;

    Camera *getCamera() const;

	unsigned int getNumLights() const;

    const Light *getLight(unsigned int light) const;

};

inline Scene::Scene()
    : camera(nullptr),
      environmentSampler(nullptr),
      environment(nullptr)
{
}

inline Scene::~Scene() {
    
}

inline void Scene::addLight(Light *light) {
    lights.push_back(light);
}

inline void Scene::setCamera(Camera *camera) {
    this->camera = camera;
}

inline void Scene::setEnvironment(Image<float, 4> *environment) {
    this->environment = environment;
}

inline void Scene::setEnvironmentSampler(Sampler *sampler) {
    this->environmentSampler = sampler;
}

inline Material *Scene::getMaterial(unsigned int id) const {
    return materials[id];
}

inline unsigned int Scene::getNumMaterials() const {
	return materials.size();
}

inline util::vector<Triangle, 16> & Scene::getTriangles() {
    return triangles;
}

inline const Triangle *Scene::getTriangle(unsigned int id) const {
    return &triangles[id];
}

inline const Image<float, 4> *Scene::getEnvironment() const {
    return environment;
}

inline const Sampler *Scene::getEnvironmentSampler() const {
    return environmentSampler;
}

inline Camera *Scene::getCamera() const {
    return camera;
}

inline unsigned int Scene::getNumLights() const {
	return lights.size();
}

inline const Light *Scene::getLight(unsigned int light) const {
    return lights[light];
}

#endif
