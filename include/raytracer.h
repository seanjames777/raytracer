/**
 * @file raytracer.h
 *
 * @brief Raytracer main logic
 *
 * @author Sean James
 */

#ifndef _RAYTRACER_H
#define _RAYTRACER_H

#include <defs.h>
#include <rtmath.h>
#include <scene.h>
#include <image.h>
#include <kdtree.h>
#include <glimagedisplay.h>
#include <timer.h>
#include <raytracersettings.h>

#include <thread>
#include <atomic>
#include <mutex>

class Raytracer {
private:

    /** @brief Intersection test tree */
    KDTree *tree;

    /** @brief Scene */
    Scene *scene;

    /** @brief Work queue */
    std::vector<int2> blocks;

    /** @brief Work queue lock */
    std::mutex blocksLock;

    /** @brief Number of blocks unfinished */
    int blocksUnfinished;

    /** @brief Whether workers should keep working */
    bool keepWorking;

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
    void photon_map(std::vector<Photon> & photons, int num_photons) {
        std::vector<Vec3> samples;
        randSphere(samples, num_photons);

        for (int i = 0; i < samples.size(); i++) {
            Photon p(samples[i] * 5.0f, Vec3(1.0f, 1.0f, 1.0f), Vec3());
            photons.push_back(p);
        }
    }

    /**
     * @brief Project photons into the output image for visualization
     */
    void photon_vis(std::vector<Photon> & photons) {
        Mat4x4 view = Mat4x4::lookAtRH(scene->camera->getPosition(), scene->camera->getTarget(), scene->camera->getUp());
        Mat4x4 projection = Mat4x4::perspectiveRH(scene->camera->getFOV(), scene->camera->getAspectRatio(), 0.01f, 100.0f);
        Mat4x4 vp = projection * view;

        for (int i = 0; i < photons.size(); i++) {
            Photon p = photons[i];

            Vec4 pos = vp * Vec4(p.position, 1);
            pos = pos / pos.w;
            pos = (pos / 2.0) + 0.5;

            int x = (int)(pos.x * (scene->output->getWidth() - 1));
            int y = (int)(pos.y * (scene->output->getHeight() - 1));

            if (x < 0 || x >= scene->output->getWidth())
                continue;

            if (y < 0 || y >= scene->output->getHeight())
                continue;

            scene->output->setPixel(x, y, Vec4(1, 1, 1, 1));
        }
    }

    /**
     * @brief Shade the result of a view ray/object collision
     *
     * @param result Collision information
     * @param depth  Recursion depth
     */
    Vec3 shade(Ray ray, Collision *result, int depth) {
        Vec3 color = Vec3(0.0f, 0.0f, 0.0f);

        if (depth > settings.maxDepth)
            return color;

        Material *material = scene->materialMap[result->polygonID];

        // TODO: might be better to pass ray by pointer or something
        color = material->shade(ray, result, scene, this, depth);

        return color;
    }

    /**
     * @brief Entry point for a worker thread
     */
    void worker_thread() {
        while(keepWorking) {
            blocksLock.lock();

            if (blocks.size() == 0) {
                blocksLock.unlock();
                continue;
            }

            int2 args = blocks[0];
            blocks.erase(blocks.begin(), blocks.begin() + 1);
            blocksLock.unlock();

            int x0 = settings.blockSize * args.x;
            int y0 = settings.blockSize * args.y;

            int width = scene->output->getWidth();
            int height = scene->output->getHeight();

            float sampleOffset = 1.0f / (float)(settings.pixelSamples + 1);
            float sampleContrib = 1.0f / (float)(settings.pixelSamples * settings.pixelSamples);

            for (int y = y0; y < y0 + settings.blockSize; y++) {
                for (int x = x0; x < x0 + settings.blockSize; x++) {
                    if (x >= width || y >= height)
                        continue;

                    Vec3 color = Vec3(0.0f, 0.0f, 0.0f);

                    for (int p = 0; p < settings.pixelSamples; p++) {
                        float v = (float)(y + p * sampleOffset) / (float)height;

                        for (int q = 0; q < settings.pixelSamples; q++) {
                            float u = (float)(x + q * sampleOffset) / (float)width;

                            Ray r = scene->camera->getViewRay(u, v);

                            Collision result, temp;
                            result.distance = INFINITY32F;

                            Vec3 sampleColor = getEnvironment(r.direction);

                            if (tree->intersect(r, &result, 0.0f, false))
                                sampleColor = shade(r, &result, 1);

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

                    scene->output->setPixel(x, y, Vec4(color, 1.0f));
                }
            }

            blocksLock.lock();
            blocksUnfinished--;
            blocksLock.unlock();
        }
    }

    void clearChecker() {
        for (int y = 0; y < scene->output->getHeight(); y++)
            for (int x = 0; x < scene->output->getWidth(); x++) {
                int bx = (y / 16) % 2;
                int by = (x / 16) % 2;
                int b = bx ^ by;
                float c = b > 0 ? 0.8f : 0.7f;
                scene->output->setPixel(x, y, Vec4(c, c, c, 1.0f));
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
        std::vector<PolygonAccel *> accelPtrs;

        for (int i = 0; i < scene->polyAccels.size(); i++)
            accelPtrs.push_back(&scene->polyAccels[i]);

        tree = new KDTree(accelPtrs);

        srand((unsigned)time(0));
    }

    /**
     * @brief Start worker threads
     */
    void startThreads() {
        keepWorking = true;

        int nThreads = settings.numThreads;

        if (nThreads == 0)
            nThreads = std::thread::hardware_concurrency();

        for (int i = 0; i < nThreads; i++)
            workers.push_back(std::thread(std::bind(&Raytracer::worker_thread, this)));

        printf("Started %d worker threads\n", nThreads);
    }

    /**
     * @brief Stop worker threads
     */
    void stopThreads() {
        keepWorking = false;

        for (auto& worker : workers)
            worker.join();

        workers.clear();
    }

    /**
     * @brief Render the scene into the scene's output image
     */
    void render(GLImageDisplay *display) {
        int nBlocksW = (scene->output->getWidth() + settings.blockSize - 1) / settings.blockSize;
        int nBlocksH = (scene->output->getHeight() + settings.blockSize - 1) / settings.blockSize;

        clearChecker();

        blocksLock.lock();
        for (int y = 0; y < nBlocksH; y++)
            for (int x = 0; x < nBlocksW; x++)
                blocks.push_back(int2(x, y));

        blocksUnfinished = blocks.size();
        blocksLock.unlock();

        if (display != NULL) {
            while (blocksUnfinished > 0) {
                display->refresh();
                usleep(33000);
            }
        }

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
    float getShadow(Collision *result, CollisionEx *resultEx, Light *light) {
        if (!light->castsShadows())
            return 1.0f;

        Vec3 origin = resultEx->position + resultEx->normal * .01f;
        
        float shadow = 0.0f;

        std::vector<Vec3> samples;
        light->getShadowDir(origin, samples, settings.shadowSamples);
        int nSamples = samples.size();

        for (int i = 0; i < nSamples; i++) {
            Vec3 dir = samples[i];
            float maxDist = dir.len();
            dir = dir / maxDist;

            Ray shadow_ray(origin, dir);

            Collision shadow_result;
            if (!tree->intersect(shadow_ray, &shadow_result, maxDist, true))
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
    float getAmbientOcclusion(Collision *result, CollisionEx *resultEx) {
        Vec3 origin = resultEx->position + resultEx->normal * .001f;

        float occlusion = 1.0f;

        int sqrtNSamples = sqrt(settings.occlusionSamples);
        int nSamples = sqrtNSamples * sqrtNSamples;

        std::vector<Vec3> samples;
        randHemisphereCos(resultEx->normal, samples, sqrtNSamples);

        for (int i = 0; i < nSamples; i++) {
            Ray occl_ray(origin, samples[i]);

            Collision occl_result;
            if (tree->intersect(occl_ray, &occl_result, settings.occlusionDistance, true))
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
    Vec3 getIndirectLighting(Collision *result, CollisionEx *resultEx, int depth) {
        // TODO: normal
        Vec3 origin = resultEx->position + resultEx->normal * .001f;

        Vec3 color = Vec3(0, 0, 0);

        int sqrtNSamples = sqrt(settings.indirectSamples);
        int nSamples = sqrtNSamples * sqrtNSamples;

        std::vector<Vec3> samples;
        randHemisphereCos(resultEx->normal, samples, sqrtNSamples);

        for (int i = 0; i < nSamples; i++) {
            Ray ind_ray(origin, samples[i]);

            Collision ind_result;
            if (tree->intersect(ind_ray, &ind_result, 50.0f, false))
                color += shade(ind_ray, &ind_result, depth + 1) * (1.0f / nSamples) *
                    (1.0f - SATURATE(ind_result.distance / 50.0f));
        }

        return color;
    }

    /**
     * @brief Sample the environment map, if there is one. Otherwise, returns the background
     * color (TODO).
     *
     * @param norm Direction to sample the environment
     */
    Vec3 getEnvironment(Vec3 norm) {
        if (scene->environment != NULL) {
            Vec4 sample = scene->environment->getPixel(norm);
            return Vec3(sample.x, sample.y, sample.z);
        }

        return Vec3(0, 0, 0);
    }

    /**
     * @brief Get the environment reflection at a point across a normal
     */
    Vec3 getEnvironmentReflection(Collision *result, CollisionEx *resultEx) {
        return getEnvironment(resultEx->ray.direction.reflect(resultEx->normal));
    }

    /**
     * @brief Get the environment refraction at a point across a normal, assuming the environment
     * is filled with air.
     *
     * @param result Collision information
     * @param ior    Index of refraction of material
     */
    Vec3 getEnvironmentRefraction(Collision *result, float ior, CollisionEx *resultEx) {
        return getEnvironment(resultEx->ray.direction.refract(resultEx->normal, 1.0f, ior));
    }
};

#endif