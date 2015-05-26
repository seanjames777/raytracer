/**
 * @file shader/pbrshader.h
 *
 * @brief Physically based shader
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __PBRSHADER_H
#define __PBRSHADER_H

#include <shader/shader.h>

// TODO: "Low discrepency" sampling?

// Note:
// http://renderwonk.com/publications/s2010-shading-course/hoffman/s2010_physically_based_shading_hoffman_a_notes.pdf
//
// Rendering equation: Lo(v) = Integrate[f(l,v) * Li(l) * (n.l), {Omega}]
//     f(l,v) is BRDF, v is outgoing light direction
//     Lo and Li are outgoing and incoming light
//     Omega is hemisphere of possible light directions and l is incoming light direction
//     Note that the BRDF should be normalized such that Lo(v) <= 1 for all v
//
// For punctual light sources and isotropic BRDF, Li(l) is 0 for all but 1 direction l, so:
//     Integrate[f(l,v) * Li(l) * (n.l), {Omega}] = Pi * f(l,v) * Li(l),
//     so 1/Pi terms are not needed in normalized BRDFs
//
// Lambertian BRDF:
//   Integrate[Integrate[Cos[Theta] Sin[Theta], {Theta, 0, Pi/2}], {Phi, 0, 2 Pi}]
//   = Pi, so normalization constant is 1 / Pi
//
// Phong Specular BRDF: (brightest looking directly at lobe)
//   Integrate[
//      Integrate[Cos[Theta]^n Cos[Theta] Sin[Theta], {Theta, 0, Pi/2}],
//   {Phi, 0, 2 Pi}]
//   = (2 Pi) / (2 + n), so normalization constant is (2+n)/(2 Pi)
//
// Rendering equation is approximated with Monte Carlo integration:
//   Lo(v) ~ (1/N) Sum[(Li(l) f(l,v) (n.l)) / p(l), {k, 1, N}]
//   For importance sampling we should use a cosine distribution when integrating over
//   a hemisphere. If we know more about the BRDF, we can use better importance sampling--along
//   the phong lobe, for example.

/**
 * @brief Physically based shader
 */
class RT_EXPORT PBRShader : public Shader {
private:

public:

    /**
     * @brief Constructor
     */
    PBRShader();

    /**
     * @brief Destructor
     */
    virtual ~PBRShader();

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
