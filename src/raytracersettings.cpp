/**
 * @file raytracersettings.cpp
 *
 * @author Sean James
 */

#include <raytracersettings.h>

RaytracerSettings::RaytracerSettings()
    : blockSize(32),
      pixelSamples(1),
      shadowSamples(1),
      occlusionSamples(0),
      maxDepth(2),
      indirectSamples(0),
      numThreads(0),
      width(1024),
      height(1024)
{
}