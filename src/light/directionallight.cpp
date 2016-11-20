/**
 * @file light/directionallight.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <light/directionallight.h>

DirectionalLight::DirectionalLight(vec3 direction, vec3 color, bool shadow)
    : direction(direction),
      color(color),
      shadow(shadow)
{
}

vec3 DirectionalLight::getDirection(const vec3 & pos) {
    return direction;
}

vec3 DirectionalLight::getColor(const vec3 & pos) {
    return color;
}

bool DirectionalLight::castsShadows() {
    return shadow;
}

vec3 DirectionalLight::getShadowDir(const vec3 & at) {
    return -direction;
}
