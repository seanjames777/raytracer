/**
 * @file light/directionallight.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <light/directionallight.h>

DirectionalLight::DirectionalLight(float3 direction, float3 color, bool shadow)
    : direction(direction),
      color(color),
      shadow(shadow)
{
}

float3 DirectionalLight::getDirection(const float3 & pos) {
    return direction;
}

float3 DirectionalLight::getColor(const float3 & pos) {
    return color;
}

bool DirectionalLight::castsShadows() {
    return shadow;
}

float3 DirectionalLight::getShadowDir(const float3 & at) {
    return -direction;
}
