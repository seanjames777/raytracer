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

#include <light.h>
#include <polygon.h>
#include <image.h>
#include <memory>
#include <kdtree/kdtree.h>

class Raytracer;
class Scene;

/**
 * @brief Base class for all materials
 */
class RT_EXPORT Material {
public:

    Material();

    ~Material();

    virtual vec3 shade(
        util::stack<KDStackFrame> & stack,
        const Ray & ray,
        Collision *result,
        Scene *scene,
        Raytracer *raytracer,
        int depth) = 0;
};

/**
 * @brief Phong material
 */
class RT_EXPORT PhongMaterial : public Material {
private:

    /** @brief Ambient light color */
    vec3 ambient;

    /** @brief Diffuse light color */
    vec3 diffuse;

    /** @brief Specular light color */
    vec3 specular;

    /** @brief Specular power */
    float specularPower;

    /** @brief Reflection amount */
    float reflection;

    /** @brief Refraction amount */
    float refraction;

    /** @brief Index of refraction */
    float ior;

    std::shared_ptr<Image> diffuse_texture;
    std::shared_ptr<Sampler> diffuse_sampler;

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
    PhongMaterial(vec3 ambient, vec3 diffuse, vec3 specular, float specularPower, float reflection,
        float refraction, float ior, std::shared_ptr<Sampler> diffuse_sampler,
        std::shared_ptr<Image> diffuse_texture);

    /**
     * @brief Shade the surface for a given collision and light source. Will be called for each
     * light source and mixed externally with shadows, reflections, etc.
     */
    virtual vec3 shade(
        util::stack<KDStackFrame> & stack,
        const Ray & ray,
        Collision *result,
        Scene *scene,
        Raytracer *raytracer,
        int depth) override;

};

/**
 * @brief Physically based material
 */
class RT_EXPORT PBRMaterial : public Material {
private:

public:

    PBRMaterial();

    /**
     * @brief Shade the surface for a given collision and light source. Will be called for each
     * light source and mixed externally with shadows, reflections, etc.
     */
    virtual vec3 shade(
        util::stack<KDStackFrame> & stack,
        const Ray & ray,
        Collision *result,
        Scene *scene,
        Raytracer *raytracer,
        int depth) override;

};

#endif
