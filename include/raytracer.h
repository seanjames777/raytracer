/**
 * @file raytracer.h
 *
 * @brief Raytracer main logic
 *
 * @author Sean James
 */

#ifndef _RAYTRACER_H
#define _RAYTRACER_H

#include <rtmath.h>
#include <scene.h>
#include <image.h>
#include <kdtree/kdtree.h>
#include <kdtree/kdsahbuilder.h>
#include <kdtree/kdmedianbuilder.h>
#include <glimagedisplay.h>
#include <timer.h>
#include <raytracersettings.h>
#include <unistd.h>
#include <iostream>

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

    std::atomic<int> currBlockID;
    int nBlocks;
    int nBlocksW;
    int nBlocksH;

    /** @brief Worker threads */
    std::vector<std::thread> workers;

    /** @brief Raytracer settings */
    RaytracerSettings settings;

    /**
     * @brief Emit photons into the scene and track their bounces
     *
     * @param photons     Vector which will be filled with photons
     * @param num_photons Number of photons to emit
     */
    /*void photon_map(std::vector<Photon> & photons, int num_photons) {
        std::vector<vec3> samples;
        randSphere(samples, num_photons);

        for (int i = 0; i < samples.size(); i++) {
            Photon p(samples[i] * 5.0f, vec3(1.0f, 1.0f, 1.0f), vec3());
            photons.push_back(p);
        }
    }*/

    /**
     * @brief Project photons into the output image for visualization
     */
    void photon_vis(std::vector<Photon> & photons) {
        mat4x4 view = lookAtRH(scene->camera->getPosition(), scene->camera->getTarget(), scene->camera->getUp());
        mat4x4 projection = perspectiveRH(scene->camera->getFOV(), scene->camera->getAspectRatio(), 0.01f, 100.0f);
        mat4x4 vp = projection * view;

        for (int i = 0; i < photons.size(); i++) {
            Photon p = photons[i];

            vec4 pos = vp * vec4(p.position, 1);
            pos = pos / pos.w;
            pos = (pos / 2.0f) + 0.5f;

            int x = (int)(pos.x * (scene->output->getWidth() - 1));
            int y = (int)(pos.y * (scene->output->getHeight() - 1));

            if (x < 0 || x >= scene->output->getWidth())
                continue;

            if (y < 0 || y >= scene->output->getHeight())
                continue;

            scene->output->setPixel(x, y, vec4(1, 1, 1, 1));
        }
    }

    /**
     * @brief Shade the result of a view ray/object collision
     *
     * @param result Collision information
     * @param depth  Recursion depth
     */
    vec3 shade(KDStack & kdStack, const Ray & ray, Collision *result, int depth) {
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
        KDStack kdStack;

        while(true) {
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

                            Ray r = scene->camera->getViewRay(u, v);

                            Collision result, temp;
                            result.distance = INFINITY32F;

                            vec3 sampleColor = getEnvironment(r.direction);

                            if (tree->intersect(kdStack, r, result, 0.0f, false))
                                sampleColor = shade(kdStack, r, &result, 1);

                            /*bool found = false;
                            for (int i = 0; i < scene->polys.size(); i++)
                                if (scene->polyAccels[i].intersects(r, &temp) &&
                                    (temp.distance < result.distance || !found))
                                {
                                    result = temp;
                                    found = true;
                                }

                            if (found)
                                sampleColor = shade(r, &result, 1);*/

                            color += sampleColor * sampleContrib;
                        }
                    }

                    scene->output->setPixel(x, y, vec4(color, 1.0f));
                }
            }
        }
    }

    void clearChecker() {
        vec4 *pixels = new vec4[scene->output->getWidth() * scene->output->getHeight()];
        int idx = 0;

        for (int y = 0; y < scene->output->getHeight(); y++)
            for (int x = 0; x < scene->output->getWidth(); x++) {
                int bx = (y / 16) % 2;
                int by = (x / 16) % 2;
                int b = bx ^ by;
                float c = b > 0 ? 0.8f : 0.5f;

                pixels[idx++] = vec4(c, c, c, 1.0f);
            }

        scene->output->setPixels((float *)pixels);
        delete [] pixels;
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
        //KDSAHBuilder builder;
        KDMedianBuilder builder;
        tree = builder.build(scene->triangles);

        srand((unsigned)time(0));
    }

    // TODO: destroy

    void printProgress(int nBlocksW, int nBlocksH, bool clear) {
        const int max_count = 26;

        if (clear) {
            for (int i = 0; i < max_count + 3; i++)
                std::cout << (unsigned char)8;
        }

        std::cout << "[";

        // TODO
        float progress = (float)currBlockID.load() / (float)(nBlocksH * nBlocksW);
        if (progress > 1.0f)
            progress = 1.0f;
        if (progress < 0.0f)
            progress = 0.0f;

        int count = (int)(progress * (float)max_count);

        for (int i = 0; i < count; i++)
            std::cout << "\u2587";

        for (int i = 0; i < max_count - count; i++)
            std::cout << " ";

        std::cout << " ]" << std::flush;
    }

    /**
     * @brief Render the scene into the scene's output image
     */
    void render(GLImageDisplay *display) {
        nBlocksW = (scene->output->getWidth() + settings.blockSize - 1) / settings.blockSize;
        nBlocksH = (scene->output->getHeight() + settings.blockSize - 1) / settings.blockSize;
        nBlocks = nBlocksW * nBlocksH;
        currBlockID.store(0);

        clearChecker();

        int nThreads = settings.numThreads;

        if (nThreads == 0)
            nThreads = std::thread::hardware_concurrency();

        for (int i = 0; i < nThreads; i++)
            workers.push_back(std::thread(std::bind(&Raytracer::worker_thread, this)));

        printf("Started %d worker threads\n", nThreads);

        int time = 0;

        printProgress(nBlocksW, nBlocksH, false);

        const int refresh_ms = 100;

        if (display != NULL) {
            while (currBlockID.load() < nBlocks) {
                display->refresh();
                usleep(refresh_ms * 1000);
                time += refresh_ms;

                if (time > 250) {
                    printProgress(nBlocksW, nBlocksH, true);
                    time = 0;
                }
            }
        }

        for (auto& worker : workers)
            worker.join();

        workers.clear();

        printProgress(nBlocksW, nBlocksH, true);
        std::cout << std::endl;

        //std::vector<Photon> photons;
        //photon_map(photons, 50);
        //photon_vis(photons);

        display->refresh();
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
    float getShadow(KDStack & kdStack, const vec3 & origin, Light *light) {
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
    float getAmbientOcclusion(KDStack & kdStack, const vec3 & origin, const vec3 & normal) {
        float occlusion = 1.0f;

        int sqrtNSamples = sqrt(settings.occlusionSamples);
        int nSamples = sqrtNSamples * sqrtNSamples;

        vec3 samples[MAX_AO_SAMPLES]; // TODO
        randHemisphereCos(normal, samples, sqrtNSamples);

        for (int i = 0; i < nSamples; i++) {
            Ray occl_ray(origin, samples[i]);

            Collision occl_result;
            if (tree->intersect(kdStack, occl_ray, occl_result, settings.occlusionDistance, true))
                occlusion -= (1.0f / nSamples) * (1.0f - SATURATE(occl_result.distance /
                    settings.occlusionDistance));
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
                    (1.0f - SATURATE(ind_result.distance / 50.0f));
        }

        return color;
    }*/

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
