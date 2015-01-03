/**
 * @file directionallight.cpp
 *
 * @author Sean James
 */

#include "directionallight.h"

DirectionalLight::DirectionalLight(vec3 direction, vec3 color, bool shadow)
    : direction(direction),
      color(color),
      shadow(shadow)
{
}

vec3 DirectionalLight::getDirection(vec3 pos) {
    return direction;
}

vec3 DirectionalLight::getColor(vec3 pos) {
    return color;
}

bool DirectionalLight::castsShadows() {
    return shadow;
}

vec3 DirectionalLight::getShadowDir(vec3 at) {
    return -direction;
}
