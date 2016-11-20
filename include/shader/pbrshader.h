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
//
// http://renderwonk.com/publications/s2010-shading-course/hoffman/s2010_physically_based_shading_hoffman_a_notes.pdf
// Physically Based Rendering Chapter 5
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
//   = Pi, so normalization constant (when integrating over hemisphere) is 1 / Pi
//
// Phong Specular BRDF: (brightest looking directly at lobe)
//   Integrate[
//      Integrate[Cos[Theta]^n Cos[Theta] Sin[Theta], {Theta, 0, Pi/2}],
//   {Phi, 0, 2 Pi}]
//   = (2 Pi) / (2 + n), so normalization constant is (2+n)/(2 Pi)
//
//
// Units:
//   Flux/Phi/Power = Total energy passing through area. Joules/second == Watts
//   Irradiance/E = Density of flux leaving surface = dPhi / dA. Watts/m^2
//   Radiant Exitance/M = Density of flux arriving at surface = dPhi / dA. Watts/m^2
//   Solid angle = projected area on unit sphere. Steradians
//   Intensity/I = flux density per solid angle/directional distribution of light = dPhi/dw
//   Radiance/L = dPhi/(dw dA_perp) = Light arriving at surface as solid angle and projected area
//     become small
//
// Spherical Coordinates:
//   x = sin(theta) cos(phi)
//   y = sin(theta) sin(phi)
//   z = cos(theta)
//
// Integral Over Solid Angle:    d(w) = sin(theta) d(theta) d(phi), 0 <= theta <= pi/2, 0 <= phi </ 2 pi
// Integral Over Projected Area: d(w) = dA cos(theta) / r^2, in Integral[f(l,v) * Li(l) * (n.l) dw]
//
// Rendering equation is approximated with Monte Carlo integration: TODO
//   Lo(v) ~ (1/N) Sum[(Li(l) f(l,v) (n.l)) / p(l), {k, 1, N}]
//   For importance sampling we should use a cosine distribution when integrating over
//   a hemisphere. If we know more about the BRDF, we can use better importance sampling--along
//   the phong lobe, for example.
//
// Monte Carlo integration:
//   Can approximate  integral[f(x), dx]
//                  = integral[f(x) * p(x) / p(x), dx]
//                  = E[f(x) / p(x)]
//                  = (1 / N) * sum[f(x_i) / p(x_i), { x, 0, N }]
//
//   Example: cosine weighted diffuse interreflection: p(x) = cos(theta) / pi (sum to 1)
//      integral[f(x_i) L_i cos(theta) / p(x_i)]
//    = integral[c / pi L_i cos(theta) / (cos(theta) / pi)]
//    = c integral[L_i]
//    = (c / N) Sum[L_i, {i,0,N}]

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
    virtual float3 shade(
		KDStackFrame    * stack,
		KDTree          * tree,
		int               depth,
        const Ray       & ray,
        const Collision & result,
        Scene           * scene,
        Raytracer       * raytracer) const override;

};

#endif
