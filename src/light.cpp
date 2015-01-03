/**
 * @file light.cpp
 *
 * @author Sean James
 */

#include "light.h"

Photon::Photon(vec3 position, vec3 power, vec3 direction)
    : position(position),
      power(power),
      direction(direction)
{
}
