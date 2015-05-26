/**
 * @file shader/brdf.h
 *
 * @brief BRDF base class
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __BRDF_H
#define __BRDF_H

/**
 * @brief BRDF base class
 */
class BRDF {
public:

    BRDF() {
    }

    virtual ~BRDF() {
    }

    /**
     * @brief Evaluate the BRDF
     */
    virtual vec3 eval(const Vertex & v, const vec3 & eyeDir, const vec3 & lightDir) const = 0;
};

class LambertBRDF {
private:

    vec3 k_diff;

public:

    LambertBRDF(const vec3 & k_diff)
        : k_diff(k_diff / (float)M_PI)
    {
    }

    virtual ~LambertBRDF() {
    }

    virtual vec3 eval(const Vertex & v, const vec3 & eyeDir, const vec3 & lightDir) const override {
        return k_diff;
    }
};

class PhongBRDF {
private:

    vec3  k_diff;
    vec3  k_spec;
    float specpow;

public:

    PhongBRDF(
        const vec3 & k_diff,
        const vec3 & k_spec,
        float        specpow)
        : k_diff(k_diff / (float)M_PI),
          k_spec((2.0f * specpow) / (2.0f * (float)M_PI) * k_spec),
          specpow(specpow)
    {
    }

    virtual ~PhongBRDF() {
    }

    virtual vec3 eval(const Vertex & v, const vec3 & eyeDir, const vec3 & lightDir) const override {
        vec3 r = reflect(lightDir, v.normal);
        return k_diff + k_spec * powf(saturate(dot(r, eyeDir)), specpow);
    }
};

#endif
