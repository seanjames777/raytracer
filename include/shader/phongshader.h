/**
 * @file shader/phongshader.h
 *
 * @brief Phong shader
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __PHONGSHADER_H
#define __PHONGSHADER_H

#include <image/image.h>
#include <image/sampler.h>
#include <shader/shader.h>

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

    /** @brief Diffuse texture */
    Image<float, 3> *diffuseTexture;

    /** @brief Diffuse texture sampler */
    Sampler *diffuseSampler;

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
    PhongShader(
        const vec3      & ambient,
        const vec3      & diffuse,
        const vec3      & specular,
        float             specularPower,
        float             reflection,
        float             refraction,
        float             ior,
        Image<float, 3> * diffuseTexture,
        Sampler         * diffuseSampler);

    virtual ~PhongShader();

    /**
     * @brief Shade the surface for a given collision and light source. Will be called for each
     * light source and mixed externally with shadows, reflections, etc.
     */
    virtual vec3 shade(
        RayBuffer       & rayBuff,
        const Ray       & ray,
        const Collision & result,
        Scene           * scene,
        Raytracer       * raytracer) const override;

};

#endif
