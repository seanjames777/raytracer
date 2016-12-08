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
#include <image/image.h>

/**
 * @brief Base class for all Materials
 */
class RT_EXPORT Material {
private:

	Image<float, 4> *normalTexture;
    Image<float, 4> *transparentTexture;
    float            opacity;

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

	virtual float getReflectivity() const = 0;

	Image<float, 4> *getNormalTexture() const {
		return normalTexture;
	}

	void setNormalTexture(Image<float, 4> *texture) {
		normalTexture = texture;
	}

    Image<float, 4> *getTransparentTexture() const {
        return transparentTexture;
    }

    void setTransparentTexture(Image<float, 4> *texture) {
        transparentTexture = texture;
    }

    float getOpacity() const {
        return opacity;
    }

    void setOpacity(float opacity) {
        this->opacity = opacity;
    }
};

inline Material::Material()
	: normalTexture(nullptr),
      transparentTexture(nullptr),
      opacity(1.0f)
{
}

inline Material::~Material() {
}

#endif
