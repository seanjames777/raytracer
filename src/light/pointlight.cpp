/**
 * @file light/pointlight.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <light/pointlight.h>
#include <math/sampling.h>

PointLight::PointLight(const float3 & position, float radius, const float3 & radiance, bool shadow)
    : position(position),
      radius(radius),
      radiance(radiance),
      shadow(shadow)
{
}

// TODO: Shortcut for delta distributions
void PointLight::sample(const float3 & uv, const float3 & p, float3 & wo, float & r, float3 & Lo) const {
    float3 samplePosition = radius * normalize(uv) + position;

	wo = samplePosition - p;
	r = length(wo);
	wo = wo / r; // TODO /=

	Lo = radiance / (r * r);
}

bool PointLight::castsShadows() const {
    return shadow;
}
