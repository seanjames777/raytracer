/**
 * @file raytracer.h
 *
 * @brief Raytracer main logic
 *
 * @author Sean James
 */

#ifndef _RAYTRACER_H
#define _RAYTRACER_H

#include <scene.h>
#include <image.h>
#include <kdtree/kdtree.h>
#include <kdtree/kdsahbuilder.h>
#include <kdtree/kdmedianbuilder.h>
#include <timer.h>
#include <raytracersettings.h>
#include <iostream>
#include <math/sampling.h>
#include <math/macro.h>

#include <thread>
#include <atomic>
#include <mutex>

// TODO:
//     - Tiled backbuffer
//     - Lock-free indexing
//     - Per-mesh materials, sorted, instaed of poly -> naterial map

class Raytracer {
private:

    /** @brief Intersection test tree */
    KDTree *tree;

    /** @brief Scene */
    Scene *scene;

    std::atomic_int currBlockID;
    std::atomic_int blocksRemaining;
    int nBlocks;
    int nBlocksW;
    int nBlocksH;
    bool should_shutdown;

    /** @brief Worker threads */
    std::vector<std::thread> workers;

    /** @brief Raytracer settings */
    RaytracerSettings settings;

    /**
     * @brief Shade the result of a view ray/object collision
     *
     * @param result Collision information
     * @param depth  Recursion depth
     */
    vec3 shade(util::stack<KDStackFrame> & kdStack, const Ray & ray, Collision *result, int depth) {
        vec3 color = vec3(0.0f, 0.0f, 0.0f);

        if (depth > settings.maxDepth)
            return color;

        Material *material = scene->materialMap[result->triangle_id];

        // TODO: might be better to pass ray by pointer or something
        color = material->shade(kdStack, ray, result, scene, this, depth);

        return color;
    }

    /**
     * @brief Entry point for a worker thread
     */
    void worker_thread() {
        // Allocate a reusable KD traversal stack for each thread
        util::stack<KDStackFrame> kdStack;

        while(!should_shutdown) {
            int blockID = currBlockID++;

            if (blockID >= nBlocks)
                break;

            int y = blockID / nBlocksW;
            int x = blockID % nBlocksW;

            int x0 = settings.blockSize * x;
            int y0 = settings.blockSize * y;

            int width = scene->output->getWidth();
            int height = scene->output->getHeight();

            float sampleOffset = 1.0f / (float)(settings.pixelSamples + 1);
            float sampleContrib = 1.0f / (float)(settings.pixelSamples * settings.pixelSamples);

            for (int y = y0; y < y0 + settings.blockSize; y++) {
                for (int x = x0; x < x0 + settings.blockSize; x++) {
                    if (x >= width || y >= height)
                        continue;

                    vec3 color = vec3(0.0f, 0.0f, 0.0f);

                    for (int p = 0; p < settings.pixelSamples; p++) {
                        float v = (float)(y + p * sampleOffset) / (float)height;

                        for (int q = 0; q < settings.pixelSamples; q++) {
                            float u = (float)(x + q * sampleOffset) / (float)width;

                            Ray r = scene->camera->getViewRay(vec2(u, v));

                            vec3 sampleColor = getEnvironment(r.direction);

                            Collision result;

                            if (tree->intersect(kdStack, r, result, 0.0f, false))
                                sampleColor = shade(kdStack, r, &result, 1);

                            color += sampleColor * sampleContrib;
                        }
                    }

                    scene->output->setPixel(x, y, vec4(color, 1.0f));
                }
            }

            blocksRemaining--;
        }
    }

public:

    /**
     * @brief Constructor
     *
     * @param scene Scene to render
     */
    Raytracer(RaytracerSettings settings, Scene *scene)
        : settings(settings),
          scene(scene),
          tree(NULL)
    {
        srand((unsigned)time(0));
    }

    // TODO: destroy

    /**
     * @brief Render the scene into the scene's output image
     */
    void render() {
        should_shutdown = false;

        KDSAHBuilder builder;
        //KDMedianBuilder builder;
        tree = builder.build(scene->triangles);

        nBlocksW = (scene->output->getWidth() + settings.blockSize - 1) / settings.blockSize;
        nBlocksH = (scene->output->getHeight() + settings.blockSize - 1) / settings.blockSize;
        nBlocks = nBlocksW * nBlocksH;
        currBlockID = 0;
        blocksRemaining = nBlocks;

        int nThreads = settings.numThreads;

        if (nThreads == 0)
            nThreads = std::thread::hardware_concurrency();

        for (int i = 0; i < nThreads; i++)
            workers.push_back(std::thread(std::bind(&Raytracer::worker_thread, this)));

        printf("Started %d worker threads\n", nThreads);
    }

    bool finished() {
        return blocksRemaining == 0;
    }

    void shutdown() {
        should_shutdown = true;

        for (auto& worker : workers)
            worker.join();

        workers.clear();
    }

    /**
     * @brief Compute the shadow coverage for a collision and light pair, taking
     * settings.shadowSamples samples for area light sources.
     *
     * @param result Information about location being shaded
     * @param light  Light to calculate shadows for
     *
     * @return A floating point number ranging from 0 (fully shadowed) to 1 (fully lit)
     */
    float getShadow(util::stack<KDStackFrame> & kdStack, const vec3 & origin, Light *light) {
        if (!light->castsShadows())
            return 1.0f;

        float shadow = 0.0f;

        vec3 samples[MAX_SHADOW_SAMPLES]; // TODO
        light->getShadowDir(origin, samples, settings.shadowSamples);
        int nSamples = settings.shadowSamples;

        if (nSamples == 0)
            nSamples = 1;

        for (int i = 0; i < nSamples; i++) {
            vec3 dir = samples[i];
            float maxDist = length(dir);
            dir = dir / maxDist;

            Ray shadow_ray(origin, dir);

            Collision shadow_result;
            if (!tree->intersect(kdStack, shadow_ray, shadow_result, maxDist, true))
                shadow += 1.0f / nSamples;
        }

        return shadow;
    }

    /**
     * @brief Compute ambient-occlusion for a collision, taking settings.occlusionSamples samples
     *
     * @param result Information about location being shaded
     *
     * @return A floating point number ranging from 0 (fully occluded) to 1 (fully visible)
     */
    float getAmbientOcclusion(util::stack<KDStackFrame> & kdStack, const vec3 & origin, const vec3 & normal) {
        float occlusion = 1.0f;

        int nSamples = settings.occlusionSamples;

        vec2 jittered_2d[MAX_AO_SAMPLES * MAX_AO_SAMPLES];
        vec3 ao_samples[MAX_AO_SAMPLES * MAX_AO_SAMPLES];

        // TODO: Align samples to normal

        randJittered2D(nSamples, jittered_2d);
        mapSamplesCosHemisphere(nSamples, 1.0f, jittered_2d, ao_samples);
        alignHemisphereNormal(nSamples, ao_samples, normal);

        float sampleWeight = 1.0f / (nSamples * nSamples);
        float invMaxDist = 1.0f / settings.occlusionDistance; // tODO precalculate these kinds of things

        for (int i = 0; i < nSamples * nSamples; i++) {
            Ray occl_ray(origin, ao_samples[i]);

            Collision occl_result;
            if (tree->intersect(kdStack, occl_ray, occl_result, settings.occlusionDistance, true))
                occlusion -= (1.0f - saturate(occl_result.distance * invMaxDist)) * sampleWeight;
        }

        return occlusion;
    }

    /**
     * @brief Compute indirect lighting for a collision, taking settings.indirectSamples samples
     *
     * @param result Information about location being shaded
     */
    /*vec3 getIndirectLighting(Collision *result, CollisionEx *resultEx, int depth) {
        // TODO: normal
        vec3 origin = resultEx->position + resultEx->normal * .001f;

        vec3 color = vec3(0, 0, 0);

        int sqrtNSamples = sqrt(settings.indirectSamples);
        int nSamples = sqrtNSamples * sqrtNSamples;

        std::vector<vec3> samples;
        randHemisphereCos(resultEx->normal, samples, sqrtNSamples);

        for (int i = 0; i < nSamples; i++) {
            Ray ind_ray(origin, samples[i]);

            Collision ind_result;
            if (tree->intersect(ind_ray, &ind_result, 50.0f, false))
                color += shade(ind_ray, &ind_result, depth + 1) * (1.0f / nSamples) *
                    (1.0f - saturate(ind_result.distance / 50.0f));
        }

        return color;
    }*/

    vec3 getGlossyReflection(util::stack<KDStackFrame> & kdStack, const vec3 & origin, const vec3 & normal,
        const vec3 & refDirection, int depth)
    {
        #define MAX_GLOSSY_SAMPLES 4 // TODO

        // TODO: Handle reflection samples that fall back into object at grazing angles

        vec3 reflDir = reflect(-refDirection, normal);

        if (depth > 1) {
            // TODO: Maybe precompute a convolved environment map to at least match the glossiness,
            // or do something else entirely.
            return getEnvironment(reflect(-refDirection, normal));
        }

        int nSamples = MAX_GLOSSY_SAMPLES;

        vec2 jittered_2d[MAX_GLOSSY_SAMPLES * MAX_GLOSSY_SAMPLES];
        vec3 glossy_samples[MAX_GLOSSY_SAMPLES * MAX_GLOSSY_SAMPLES];

        randJittered2D(nSamples, jittered_2d);
        mapSamplesCosHemisphere(nSamples, 500.0f, jittered_2d, glossy_samples);
        alignHemisphereNormal(nSamples, glossy_samples, reflDir);

        float sampleWeight = 1.0f / (float)(nSamples * nSamples);

        vec3 env;

        for (int i = 0; i < nSamples * nSamples; i++) {
            Ray ind_ray(origin + normal * .001f, glossy_samples[i]);
            Collision ind_result;

            if (tree->intersect(kdStack, ind_ray, ind_result, 0.0f, false))
                env += shade(kdStack, ind_ray, &ind_result, depth + 1);
            else
                env += getEnvironment(glossy_samples[i]);
        }

        env *= sampleWeight;

        return env;
    }

    /**
     * @brief Sample the environment map, if there is one. Otherwise, returns the background
     * color (TODO).
     *
     * @param norm Direction to sample the environment
     */
    vec3 getEnvironment(const vec3 & norm) {
        if (scene->environment != NULL) {
            vec4 sample = scene->environment_sampler->sample(scene->environment.get(), norm);
            return vec3(sample.x, sample.y, sample.z);
        }

        return vec3(0, 0, 0);
    }

    /**
     * @brief Get the environment reflection at a point across a normal
     */
    vec3 getEnvironmentReflection(const vec3 & direction, const vec3 & normal) {
        return getEnvironment(reflect(direction, normal));
    }

    /**
     * @brief Get the environment refraction at a point across a normal, assuming the environment
     * is filled with air.
     *
     * @param result Collision information
     * @param ior    Index of refraction of material
     */
    vec3 getEnvironmentRefraction(const vec3 & direction, const vec3 & normal, float ior) {
        return getEnvironment(refract(direction, normal, 1.0f, ior));
    }
};

#endif
