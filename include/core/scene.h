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
#include <util/imageloader.h>
#include <util/path.h>
#include <vector>

class MeshInstance {
public:

    Mesh *mesh;
    float3 translation;
    float3 rotation;
    float3 scale;
    bool reverseWinding;

    MeshInstance(Mesh *mesh, float3 translation, float3 rotation, float3 scale, bool reverseWinding = false)
        : mesh(mesh),
          translation(translation),
          rotation(rotation),
          scale(scale),
          reverseWinding(reverseWinding)
    {
    }
};

// TODO: Might want to make some of the pointers not-pointers, i.e. just copy
//       lights into scene directly. Cache benefits and easier memory
//       management.

/**
 * @brief Scene structure, which tracks the polygons, lights, etc. in a scene
 */
class RT_EXPORT Scene {
private:

    std::vector<MeshInstance *> meshes;
    std::vector<Light *>        lights;
    Camera                     *camera;
    float3                      environmentColor;
    Image<float, 4>            *environmentMap;
    Sampler                    *environmentMapSampler;

public:

    Scene();

    virtual ~Scene();

    void addLight(Light *light);

    void addMesh(MeshInstance *instance);

    void setCamera(Camera *camera);

    void setEnvironmentColor(const float3 & color);

    void setEnvironmentMap(Image<float, 4> *environment);

    void setEnvironmentMapSampler(Sampler *sampler);

    float3 getEnvironmentColor() const;

    Image<float, 4> *addTexture(std::string name);

    const Image<float, 4> *getEnvironmentMap() const;

    const Sampler *getEnvironmentMapSampler() const;

    Camera *getCamera() const;

	unsigned int getNumLights() const;

    const Light *getLight(unsigned int light) const;

    unsigned int getNumMeshInstances() const;

    const MeshInstance *getMeshInstance(unsigned int meshInstance) const;

};

inline Scene::Scene()
    : camera(nullptr),
      environmentMapSampler(nullptr),
      environmentMap(nullptr)
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

inline void Scene::setEnvironmentColor(const float3 & color) {
    environmentColor = color;
}

inline void Scene::setEnvironmentMap(Image<float, 4> *environmentMap) {
    this->environmentMap = environmentMap;
}

inline void Scene::setEnvironmentMapSampler(Sampler *sampler) {
    this->environmentMapSampler = sampler;
}

inline float3 Scene::getEnvironmentColor() const {
    return environmentColor;
}

inline const Image<float, 4> *Scene::getEnvironmentMap() const {
    return environmentMap;
}

inline const Sampler *Scene::getEnvironmentMapSampler() const {
    return environmentMapSampler;
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

inline void Scene::addMesh(MeshInstance *mesh) {
    meshes.push_back(mesh);
}

inline unsigned int Scene::getNumMeshInstances() const {
    return meshes.size();
}

inline const MeshInstance *Scene::getMeshInstance(unsigned int meshInstance) const {
    return meshes[meshInstance];
}

#endif
