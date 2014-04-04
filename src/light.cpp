/**
 * @file light.cpp
 *
 * @author Sean James
 */

#include "light.h"

Photon::Photon(Vec3 position, Vec3 power, Vec3 direction)
    : position(position),
      power(power),
      direction(direction)
{
}