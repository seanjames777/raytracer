/**
 * @file shader.h
 *
 * @brief Shader base class
 *
 * @author Sean James
 */

#ifndef __SHADER_H
#define __SHADER_H

#include <light.h>
#include <polygon.h>
#include <image.h>
#include <memory>
#include <kdtree/kdtree.h>

class Raytracer;
class Scene;

/**
 * @brief Base class for all Shaders
 */
class RT_EXPORT Shader {
public:

    Shader();

    ~Shader();

    virtual vec3 shade(
        util::stack<KDStackFrame> & stack,
        const Ray & ray,
        Collision *result,
        Scene *scene,
        Raytracer *raytracer,
        int depth) = 0;
};

/**
 * @brief Phong Shader
 */
class RT_EXPORT PhongShader : public Shader {
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
    PhongShader(vec3 ambient, vec3 diffuse, vec3 specular, float specularPower, float reflection,
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
 * @brief Physically based Shader
 */
class RT_EXPORT PBRShader : public Shader {
private:

public:

    PBRShader();

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
