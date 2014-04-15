/**
 * @file pointlight.cpp
 *
 * @author Sean James
 */

#include "pointlight.h"

PointLight::PointLight(Vec3 position, Vec3 color, float radius, float range, float power,
    bool shadow)
    : position(position), 
      color(color),
      radius(radius),
      range2(range * range), 
      power(power), 
      shadow(shadow)
{
}

Vec3 PointLight::getDirection(Vec3 pos) {
    Vec3 dir = pos - position;
    dir.normalize();

    return dir;
}

Vec3 PointLight::getColor(Vec3 pos) {
    Vec3 diff = pos - position;
    float dist2 = diff.len2();

    float falloff = 1.0f - SATURATE(dist2 / range2);
    falloff = powf(falloff, power);

    return color * falloff;
}

bool PointLight::castsShadows() {
    return shadow;
}

void PointLight::getShadowDir(Vec3 at, std::vector<Vec3> & samples, int nSamples) {
    if (radius == 0.0f || nSamples == 1) {
        Vec3 dir = this->position - at;
        samples.push_back(dir);
        return;
    }

    int sqrtNSamples = sqrt(nSamples);
    nSamples = sqrtNSamples * sqrtNSamples;

    std::vector<Vec3> sphere;
    randSphere(sphere, sqrtNSamples);

    for (int i = 0; i < nSamples; i++) {
        Vec3 dir = (sphere[i] * radius + position) - at;
        samples.push_back(dir);
    }
}