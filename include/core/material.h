/**
 * @file core/material.h
 *
 * @brief Material base class
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MATERIAL_H
#define __MATERIAL_H

#include <core/triangle.h>

/**
 * @brief Base class for all Materials
 */
class RT_EXPORT Material {
public:

    /**
     * @brief Constructor
     */
    Material();

    /**
     * @brief Destructor
     */
    virtual ~Material();

    /**
     * @brief Evaluate the material's BRDF(s) at a collision point
     */
    virtual float3 f(
        const Vertex & interp,
        const float3 & wo,
        const float3 & wi) const = 0;
};

inline Material::Material() {
}

inline Material::~Material() {
}

#endif
