/**
 * @file raytracersettings.h
 *
 * @brief Raytracer rendering settings
 *
 * @author Sean James
 */

#ifndef _RAYTRACERSETTINGS_H
#define _RAYTRACERSETTINGS_H

#include <defs.h>

struct RaytracerSettings {
    /** @brief Image width */
    int width;

    /** @brief Image height */
    int height;

    /** @brief Block side size, in pixels. Threads render one block at a time */
    int blockSize;

    /** @brief Square root of number of samples to take per pixel */
    int pixelSamples;

    /** @brief Number of samples to take for light shadows */
    int shadowSamples;

    /** @brief Number of ambient occlusion samples */
    int occlusionSamples;

    /** @brief Maximum ambient occlusion distance */
    float occlusionDistance;

    /** @brief Maximum recursion depth */
    int maxDepth;

    /** @brief Maximum number of indirect lighting samples */
    int indirectSamples;

    /** @brief Number of threads to use, or 0 to use all available hardware threads */
    int numThreads;

    RaytracerSettings();
};

#endif
