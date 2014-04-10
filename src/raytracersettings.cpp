/**
 * @file raytracersettings.cpp
 *
 * @author Sean James
 */

#include <raytracersettings.h>

RaytracerSettings::RaytracerSettings()
    : blockSize(64),
      pixelSamples(2),
      shadowSamples(16),
      occlusionSamples(16),
      maxDepth(2),
      indirectSamples(16)
{
}
