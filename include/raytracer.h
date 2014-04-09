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
#include <bitmap.h>
#include <kdtree.h>
#include <glimagedisplay.h>
#include <timer.h>

#include <thread>
#include <atomic>
#include <mutex>

#define BLOCKSZ 64
#define FRAGSAMPLES 4
#define LIGHTSAMPLES 16
#define OCCLUSIONSAMPLES 16
#define MAXDEPTH 2
#define REFLSAMPLES 4

class Raytracer {
private:

	/** @brief Intersection test tree */
	KDTree<Shape, CollisionResult> *tree;

	/** @brief Scene */
	Scene *scene;

	/** @brief Work queue */
	std::vector<int2> blocks;

	/** @brief Work queue lock */
	std::mutex blocksLock;

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
	Vec3 shade(CollisionResult *result, int depth) {
		Vec3 color = Vec3(0.0f, 0.0f, 0.0f);

		if (depth > MAXDEPTH)
			return color;

		Material *material = scene->materialMap[result->shape];

		color = material->shade(result, scene, this);

		color.x = SATURATE(color.x);
		color.y = SATURATE(color.y);
		color.z = SATURATE(color.z);

		return color;
	}

	/**
	 * @brief Entry point for a worker thread
	 */
	void worker_thread() {
		printf("Started worker thread\n");

		blocksLock.lock();

		while(blocks.size() > 0) {
			int2 args = blocks[0];
			blocks.erase(blocks.begin(), blocks.begin() + 1);
			blocksLock.unlock();

			int x0 = BLOCKSZ * args.x;
			int y0 = BLOCKSZ * args.y;

			int width = scene->output->getWidth();
			int height = scene->output->getHeight();

			float sampleOffset = 1.0f / (float)(FRAGSAMPLES + 1);
			float sampleContrib = 1.0f / (float)(FRAGSAMPLES * FRAGSAMPLES);

			for (int y = y0; y < y0 + BLOCKSZ; y++) {
				for (int x = x0; x < x0 + BLOCKSZ; x++) {
					if (x >= width || y >= height)
						continue;

					Vec3 color = Vec3(0.0f, 0.0f, 0.0f);

					for (int p = 0; p < FRAGSAMPLES; p++) {
						float v = (float)(y + p * sampleOffset) / (float)height;

						for (int q = 0; q < FRAGSAMPLES; q++) {
							float u = (float)(x + q * sampleOffset) / (float)width;

							Ray r = scene->camera->getViewRay(u, v);

							CollisionResult result, temp;

							Vec3 sampleColor = getEnvironment(r.direction);

							if (tree->intersect(r, &result, 0.0f))
								sampleColor = shade(&result, 1);

							color += sampleColor * sampleContrib;
						}
					}

					scene->output->setPixel(x, y, Vec4(color, 1.0f));
				}
			}

			blocksLock.lock();
		}

		blocksLock.unlock();
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
	Raytracer(Scene *scene)
		: scene(scene),
		  tree(new KDTree<Shape, CollisionResult>(scene->shapes))
	{
		srand((unsigned)time(0));
	}

	/**
	 * @brief Render the scene into the scene's output image
	 */
	void render(GLImageDisplay *display) {
		Timer timer;

		int nBlocksW = (scene->output->getWidth() + BLOCKSZ - 1) / BLOCKSZ;
		int nBlocksH = (scene->output->getHeight() + BLOCKSZ - 1) / BLOCKSZ;
		
		for (int y = 0; y < nBlocksH; y++)
			for (int x = 0; x < nBlocksW; x++)
				blocks.push_back(int2(x, y));

		clearChecker();

		std::vector<std::thread> workers;

		for (int i = 0; i < std::thread::hardware_concurrency() - 1; i++)
			workers.push_back(std::thread(std::bind(&Raytracer::worker_thread, this)));

		if (display != NULL) {
			while (blocks.size() > 0) {
				display->refresh();
				usleep(33000);
			}
		}
		
		for (auto& worker : workers)
			worker.join();

		std::vector<Photon> photons;
		//photon_map(photons, 50);
		//photon_vis(photons);

		printf("Done: %f seconds\n", timer.getElapsedMilliseconds() / 1000.0);

		if (display != NULL) {
			display->refresh();
			//printf("Press any key to exit: ");
			//getchar();
		}
	}

	/**
	 * @brief Compute the shadow coverage for a collision and light pair, taking LIGHTSAMPLES
	 * samples for area light sources.
	 *
	 * @param result Information about location being shaded
	 * @param light  Light to calculate shadows for
	 *
	 * @return A floating point number ranging from 0 (fully shadowed) to 1 (fully lit)
	 */
	float getShadow(CollisionResult *result, Light *light) {
		if (!light->castsShadows())
			return 1.0f;

		Ray shadow_ray;
		shadow_ray.origin = result->position + result->normal * .001f;

		float shadow = 0.0f;

		std::vector<Vec3> samples;
		light->getShadowDir(result->position, samples, LIGHTSAMPLES);
		int nSamples = samples.size();

		for (int i = 0; i < nSamples; i++) {
			shadow_ray.direction = samples[i];

			float maxDist = shadow_ray.direction.len();
			shadow_ray.direction = shadow_ray.direction / maxDist;

			CollisionResult shadow_result;
			if (!tree->intersect(shadow_ray, &shadow_result, maxDist))
				shadow += 1.0f / nSamples;
		}

		return shadow;
	}

	/**
	 * @brief Compute ambient-occlusion for a collision, taking OCCLUSIONSAMPLES samples
	 *
	 * @param result Information about location being shaded
	 *
	 * @return A floating point number ranging from 0 (fully occluded) to 1 (fully visible)
	 */
	float getAmbientOcclusion(CollisionResult *result) {
		Ray occl_ray;
		occl_ray.origin = result->position + result->normal * .001f;

		float occlusion = 1.0f;

		int sqrtNSamples = sqrt(OCCLUSIONSAMPLES);
		int nSamples = sqrtNSamples * sqrtNSamples;

		std::vector<Vec3> samples;
		randHemisphereCos(result->normal, samples, sqrtNSamples);

		for (int i = 0; i < nSamples; i++) {
			occl_ray.direction = samples[i];

			CollisionResult occl_result;
			if (tree->intersect(occl_ray, &occl_result, 2.0f))
				occlusion -= (1.0f / nSamples) * (1.0f - SATURATE(occl_result.distance / 2.0f));
		}

		return occlusion;
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
	Vec3 getEnvironmentReflection(CollisionResult *result) {
		Vec3 origin = result->position + result->normal * .001f;
		Vec3 direction = result->ray.direction.reflect(result->normal);
		return getEnvironment(direction);
	}

	/**
	 * @brief Get the environment refraction at a point across a normal, assuming the environment
	 * is filled with air.
	 *
	 * @param result Collision information
	 * @param ior    Index of refraction of material
	 */
	Vec3 getEnvironmentRefraction(CollisionResult *result, float ior) {
		Vec3 origin = result->position + result->normal * .001f;
		Vec3 direction = result->ray.direction.refract(result->normal, 1.0f, ior);
		return getEnvironment(direction);
	}
};

#endif