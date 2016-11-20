/**
 * @file core/raytracersettings.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/raytracersettings.h>

RaytracerSettings::RaytracerSettings()
    : blockSize(32),
      pixelSamples(2),
      maxDepth(2),
      numThreads(0),
      width(1024),
      height(1024)
{
}
