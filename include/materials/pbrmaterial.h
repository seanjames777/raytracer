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
#include <image/image.h>

/**
 * @brief Physically based material
 */
class RT_EXPORT PBRMaterial : public Material {
private:

	float3 diffuseColor;
	float3 specularColor;
	float  specularPower;
	float  reflectivity;

	Image<float, 3> *diffuseTexture;
	Image<float, 3> *roughnessTexture;

public:

    PBRMaterial();

    virtual ~PBRMaterial();

    virtual float3 f(
        const Vertex & interp,
        const float3 & wo,
        const float3 & wi) const override;

	virtual float getReflectivity() const override {
		return reflectivity;
	}

	void setDiffuseColor(float3 color) {
		diffuseColor = color;
	}

	void setSpecularColor(float3 color) {
		specularColor = color;
	}

	void setSpecularPower(float power) {
		specularPower = power;
	}

	void setDiffuseTexture(Image<float, 3> *texture) {
		diffuseTexture = texture;
	}

	// TODO: Only need grayscale for this
	void setRoughnessTexture(Image<float, 3> *texture) {
		roughnessTexture = texture;
	}

	void setReflectivity(float refl) {
		reflectivity = refl;
	}

};

#endif
