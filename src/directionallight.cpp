/**
 * @file directionallight.cpp
 *
 * @author Sean James
 */

#include "directionallight.h"

DirectionalLight::DirectionalLight(Vec3 direction, Vec3 color, bool shadow)
    : direction(direction), 
      color(color), 
      shadow(shadow)
{
}

Vec3 DirectionalLight::getDirection(Vec3 pos) {
    return direction;
}

Vec3 DirectionalLight::getColor(Vec3 pos) {
    return color;
}

bool DirectionalLight::castsShadows() {
    return shadow;
}

Vec3 DirectionalLight::getShadowDir(Vec3 at) {
    return -direction;
}
