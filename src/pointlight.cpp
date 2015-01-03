/**
 * @file pointlight.cpp
 *
 * @author Sean James
 */

#include "pointlight.h"

PointLight::PointLight(vec3 position, vec3 color, float radius, float range, float power,
    bool shadow)
    : position(position),
      color(color),
      radius(radius),
      range2(range * range),
      power(power),
      shadow(shadow)
{
}

vec3 PointLight::getDirection(vec3 pos) {
    vec3 dir = normalize(pos - position);

    return dir;
}

vec3 PointLight::getColor(vec3 pos) {
    vec3 diff = pos - position;
    float dist2 = length2(diff);

    float falloff = 1.0f - SATURATE(dist2 / range2);
    falloff = powf(falloff, power);

    return color * falloff;
}

bool PointLight::castsShadows() {
    return shadow;
}

void PointLight::getShadowDir(vec3 at, std::vector<vec3> & samples, int nSamples) {
    if (radius == 0.0f || nSamples == 1) {
        vec3 dir = this->position - at;
        samples.push_back(dir);
        return;
    }

    int sqrtNSamples = sqrt(nSamples);
    nSamples = sqrtNSamples * sqrtNSamples;

    std::vector<vec3> sphere;
    randSphere(sphere, sqrtNSamples);

    for (int i = 0; i < nSamples; i++) {
        vec3 dir = (sphere[i] * radius + position) - at;
        samples.push_back(dir);
    }
}
