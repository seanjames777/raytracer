/**
 * @file core/raytracersettings.h
 *
 * @brief Raytracer rendering settings
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __RAYTRACERSETTINGS_H
#define __RAYTRACERSETTINGS_H

#include <rt_defs.h>

#define MAX_SHADOW_SAMPLES 64
#define MAX_AO_SAMPLES     64

struct RT_EXPORT RaytracerSettings {
    /** @brief Image width */
    int width;

    /** @brief Image height */
    int height;

    /** @brief Block side size, in pixels. Threads render one block at a time */
    int blockSize;

    /** @brief Square root of number of samples to take per pixel */
    int pixelSamples;

    /** @brief Maximum recursion depth */
    int maxDepth;

    /** @brief Number of threads to use, or 0 to use all available hardware threads */
    int numThreads;

    RaytracerSettings();
};

#endif
