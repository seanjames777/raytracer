/**
 * @file shader/shader.h
 *
 * @brief Shader base class
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __SHADER_H
#define __SHADER_H

#include <core/raybuffer.h>
#include <core/triangle.h>
#include <math/ray.h>

class Scene;
class Raytracer;

// TODO: Can move lots of stuff out of the core raytracer and into shader base
//       class

/**
 * @brief Base class for all Shaders
 */
class RT_EXPORT Shader {
public:

    /**
     * @brief Constructor
     */
    Shader();

    /**
     * @brief Destructor
     */
    virtual ~Shader();

    /**
     * @brief Shade at a ray/triangle intersection point
     */
    virtual float3 shade(
        RayBuffer       & rayBuff,
        const Ray       & ray,
        const Collision & result,
        Scene           * scene,
        Raytracer       * raytracer) const = 0; // TODO: can drop raytracer reference
};

inline Shader::Shader() {
}

inline Shader::~Shader() {
}

#endif
