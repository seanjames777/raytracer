/**
 * @file light/pointlight.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <light/pointlight.h>
#include <math/sampling.h>

PointLight::PointLight(float3 position, float3 color, float radius, float range, float power,
    bool shadow)
    : position(position),
      color(color),
      radius(radius),
      range2(range * range),
      power(power),
      shadow(shadow)
{
}

float3 PointLight::getDirection(const float3 & pos) {
    float3 dir = normalize(pos - position);

    return dir;
}

float3 PointLight::getColor(const float3 & pos) {
    float3 diff = pos - position;
    float dist2 = length2(diff);

    float falloff = 1.0f - saturate(dist2 / range2);
    falloff = powf(falloff, power);

    return color * falloff;
}

bool PointLight::castsShadows() {
    return shadow;
}

void PointLight::getShadowDir(const float3 & at, float3 *samples, int nSamples) {
    if (radius == 0.0f || nSamples == 0) {
        float3 dir = this->position - at;
        samples[0] = dir;
        return;
    }

    rand3D(nSamples, samples);

    for (int i = 0; i < nSamples; i++)
        samples[i] = radius * samples[i] + position - at;
}
