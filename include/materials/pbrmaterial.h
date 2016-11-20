/**
 * @file materials/pbrmaterial.h
 *
 * @brief Physically based material
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __PBRMATERIAL_H
#define __PBRMATERIAL_H

#include <core/material.h>

/**
 * @brief Physically based material
 */
class RT_EXPORT PBRMaterial : public Material {
public:

    PBRMaterial();

    virtual ~PBRMaterial();

    virtual float3 f(
        const Vertex & interp,
        const float3 & wo,
        const float3 & wi) const override;

};

#endif
