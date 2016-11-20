/**
 * @file light/pointlight.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <light/pointlight.h>
#include <math/sampling.h>

PointLight::PointLight(float3 position, float3 radiance, bool shadow)
    : position(position),
      radiance(radiance),
      shadow(shadow)
{
}

void PointLight::sample(const float3 & p, float3 & wo, float & r, float3 & Lo) const {
	wo = position - p;
	r = length(wo);
	wo /= r;

	Lo = radiance / (r * r);
}

bool PointLight::castsShadows() const {
    return shadow;
}

void PointLight::getShadowDir(const float3 & at, float3 *samples, int nSamples) {
#if 0
    if (radius == 0.0f || nSamples == 0) {
        float3 dir = this->position - at;
        samples[0] = dir;
        return;
    }

    rand3D(nSamples, samples);

    for (int i = 0; i < nSamples; i++)
        samples[i] = radius * samples[i] + position - at;
#endif
}
