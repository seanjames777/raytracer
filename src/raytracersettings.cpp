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
      occlusionDistance(10.0f),
      maxDepth(2),
      indirectSamples(0),
      numThreads(0),
      width(1024),
      height(1024)
{
}
