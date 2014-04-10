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
    /** @brief Block side size, in pixels. Threads render one block at a time */
    int blockSize;

    /** @brief Square root of number of samples to take per pixel */
    int pixelSamples;

    /** @brief Number of samples to take for light shadows */
    int shadowSamples;

    /** @brief Number of ambient occlusion samples */
    int occlusionSamples;

    /** @brief Maximum recursion depth */
    int maxDepth;

    /** @brief Maximum number of indirect lighting samples */
    int indirectSamples;

    RaytracerSettings();
};

#endif
