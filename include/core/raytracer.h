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
#include <core/raybuffer.h>
#include <core/raytracersettings.h>
#include <core/scene.h>
#include <image/image.h>
#include <kdtree/kdsahbuilder.h>
#include <kdtree/kdtree.h>
#include <rt_defs.h>
#include <thread>
#include <util/timer.h>

// TODO:
//     - Tiled backbuffer
//     - Lock-free indexing
//     - Per-mesh shaders, sorted, instaed of poly -> naterial map
//     - Interleaving various things instead of using queues, or using queues with core
//       affinity might be good since shared L1 cache or just L1 cache coherence in general
//     - Decide if the raytracer is reusable or not

/**
 * @brief Main raytracer class, which starts worker threads and coordinates the rendering
 * process.
 */
class RT_EXPORT Raytracer {
private:

    typedef std::vector<std::shared_ptr<std::thread>> threadVector;

    KDTree                  *tree;            //!< Ray/triangle intersection acceleration tree
    KDBuilderTreeStatistics  stats;           //!< Tree statistics
    Scene                   *scene;           //!< Scene to render
    int                      nBlocks;         //!< Total number of blocks to render
    int                      nBlocksW;        //!< Number of blocks horizontally
    int                      nBlocksH;        //!< Number of blocks vertically
    bool                     shouldShutdown;  //!< Whether to stop rendering
    std::atomic_int          numThreadsAlive; //!< Number of worker threads running
    std::atomic_int          currBlockID;     //!< ID of next block to render
    threadVector             workers;         //!< Worker threads
    RaytracerSettings        settings;        //!< Raytracing settings

    /**
     * @brief Entry point for a worker thread
     */
    void worker_thread(int idx, int numThreads);

public:

    /**
     * @brief Constructor
     *
     * @param scene Scene to render
     */
    Raytracer(RaytracerSettings settings, Scene *scene);

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
    void shutdown(bool waitUntilFinished);
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
    inline vec3 getEnvironment(const vec3 & norm) {
        if (scene->getEnvironment() != NULL) {
            vec4 sample = scene->getEnvironmentSampler()->sample(scene->getEnvironment(), norm);
            return vec3(sample.x, sample.y, sample.z);
        }

        return vec3(0, 0, 0);
    }

    /**
     * @brief Get the environment reflection at a point across a normal
     */
    inline vec3 getEnvironmentReflection(const vec3 & direction, const vec3 & normal) {
        return getEnvironment(reflect(direction, normal));
    }

    /**
     * @brief Get the environment refraction at a point across a normal, assuming the environment
     * is filled with air.
     *
     * @param result Collision information
     * @param ior    Index of refraction of shader
     */
    inline vec3 getEnvironmentRefraction(const vec3 & direction, const vec3 & normal, float ior) {
        return getEnvironment(refract(direction, normal, 1.0f, ior));
    }
#endif
