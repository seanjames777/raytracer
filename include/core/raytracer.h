/**
 * @file core/raytracer.h
 *
 * @brief Main raytracer implementation
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __RAYTRACER_H
#define __RAYTRACER_H

#include <atomic>
#include <core/raytracersettings.h>
#include <core/scene.h>
#include <image/image.h>
#include <kdtree/kdsahbuilder.h>
#include <kdtree/kdtree.h>
#include <rt_defs.h>
#include <thread>
#include <util/timer.h>
#include <math/sampling.h>

// TODO:
//     - Tiled backbuffer
//     - Lock-free indexing
//     - Per-mesh shaders, sorted, instaed of poly -> naterial map
//     - Interleaving various things instead of using queues, or using queues with core
//       affinity might be good since shared L1 cache or just L1 cache coherence in general
//     - Decide if the raytracer is reusable or not

enum RaytracerStat {
	RaytracerStatTotalCycles,
	RaytracerStatPrimaryEmitCycles,
	RaytracerStatPrimaryPackCycles,
	RaytracerStatPrimaryTraceCycles,
	RaytracerStatSecondaryEmitCycles,
	RaytracerStatSecondaryPackCycles,
	RaytracerStatSecondaryTraceCycles,
	RaytracerStatShadowPackCycles,
	RaytracerStatShadingPackCycles,
	RaytracerStatShadingSortCycles,
	RaytracerStatShadingCycles,
	RaytracerStatShadowTraceCycles,
	RaytracerStatUpdateFramebufferCycles,
	RaytracerStatUnaccountedCycles,
	RaytracerStatCount
};

static const char *RaytracerStatNames[] = {
	"Total Cycles",
	"Emit Primary Rays",
	"Pack Primary Rays",
	"Trace Primary Rays",
	"Emit Secondary Rays",
	"Pack Secondary Rays",
	"Trace Secondary Rays",
	"Pack Shadow Rays",
	"Pack Shading Work",
	"Sort Shading Work",
	"Shading",
	"Trace Shadow Rays",
	"Update Framebuffer",
	"Unaccounted",
	"Stat Count"
};

// TODO: align this to prevent false sharing, add option to turn it off
struct RaytracerStats {
	uint64_t stat[RaytracerStatCount];
};

struct StatTimer {
	uint64_t startTime;
	uint16_t statIndex;
};

inline StatTimer startStatTimer(uint16_t stat) {
	StatTimer timer;

#if WIN32
	timer.startTime = __rdtsc();
#endif

    timer.statIndex = stat;

	return timer;
}

inline void endStatTimer(RaytracerStats *stats, StatTimer timer) {
#if WIN32
	stats->stat[timer.statIndex] += __rdtsc() - timer.startTime;
#endif
}

/**
 * @brief Main raytracer class, which starts worker threads and coordinates the rendering
 * process.
 */
class RT_EXPORT Raytracer {
private:

    typedef std::vector<std::shared_ptr<std::thread>> threadVector;

    Image<float, 4>         *output;
    KDTree                   tree;            //!< Ray/triangle intersection acceleration tree
    KDTreeStats  _treeStats;           //!< Tree statistics
    Scene                   *scene;           //!< Scene to render
    int                      nBlocks;         //!< Total number of blocks to render
    int                      nBlocksW;        //!< Number of blocks horizontally
    int                      nBlocksH;        //!< Number of blocks vertically
    bool                     shouldShutdown;  //!< Whether to stop rendering
    std::atomic_int          numThreadsAlive; //!< Number of worker threads running
    std::atomic_int          currBlockID;     //!< ID of next block to render
    threadVector             workers;         //!< Worker threads
    RaytracerSettings        settings;        //!< Raytracing settings
	std::vector<RaytracerStats> workerStats;

    /**
     * @brief Entry point for a worker thread
     */
    void worker_thread(int idx, int numThreads, RaytracerStats *stats);

public:

    /**
     * @brief Constructor
     *
     * @param scene Scene to render
     */
    Raytracer(RaytracerSettings settings, Scene *scene, Image<float, 4> *output);

    /**
     * @brief Destructor
     */
    ~Raytracer();

    /**
     * @brief Render the scene into the scene's output image
     */
    void render();

    /**
     * @brief Whether the raytracer has finished rendering
     */
    bool finished();

    /**
     * @brief Shutdown the raytracer
     *
     * @param[in] waitUntilFinished Whether to wait until the raytracer has finished rendering
     *                              (true) or to abort immediately (false)
     */
    void shutdown(bool waitUntilFinished, RaytracerStats *stats = NULL);

    bool intersect(float2 uv, Collision & result);
};

inline bool Raytracer::finished() {
    return numThreadsAlive == 0;
}

#endif

#if 0
    /**
     * @brief Sample the environment map, if there is one. Otherwise, returns the background
     * color (TODO).
     *
     * @param norm Direction to sample the environment
     */
    inline float3 getEnvironment(const float3 & norm) {
        if (scene->getEnvironment() != NULL) {
            float4 sample = scene->getEnvironmentSampler()->sample(scene->getEnvironment(), norm);
            return float3(sample.x, sample.y, sample.z);
        }

        return float3(0, 0, 0);
    }

    /**
     * @brief Get the environment reflection at a point across a normal
     */
    inline float3 getEnvironmentReflection(const float3 & direction, const float3 & normal) {
        return getEnvironment(reflect(direction, normal));
    }

    /**
     * @brief Get the environment refraction at a point across a normal, assuming the environment
     * is filled with air.
     *
     * @param result Collision information
     * @param ior    Index of refraction of shader
     */
    inline float3 getEnvironmentRefraction(const float3 & direction, const float3 & normal, float ior) {
        return getEnvironment(refract(direction, normal, 1.0f, ior));
    }
#endif
