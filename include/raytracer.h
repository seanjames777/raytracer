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
#define NSAMPLES 4
#define MAXDEPTH 2

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

	/*Color shadeReflection(Surface *surface, CollisionResult *result, int n) {
		float reflectionAmount = surface->getReflectionAmt();

		if (scene->settings->numReflectionRays == 0 || reflectionAmount == 0.0f)
			return Color(0.0f, 0.0f, 0.0f);

		reflectionAmount *= (1.0f / scene->settings->numReflectionRays);

		Vec3 origin = result->position + result->normal * scene->settings->bias;
		Vec3 direction = result->ray.direction.reflect(result->normal);
		Ray reflectionRay(origin, direction);

		CollisionResult reflResult;

		if (intersect(reflectionRay, &reflResult, n + 1, 0.0f)) // TODO fog dist
			return shade(&reflResult, n + 1);

		return Color(0.0f, 0.0f, 0.0f);
	}*/

	/*Color shadeRefraction(Surface *surface, CollisionResult *result, int n) {
		float refractionAmount = surface->getRefractionAmt();

		if (scene->settings->numRefractionRays == 0 || refractionAmount == 0.0f)
			return Color(0.0f, 0.0f, 0.0f);

		refractionAmount *= (1.0f / scene->settings->numRefractionRays);

		Vec3 origin = result->position + result->ray.direction * scene->settings->bias;
		Vec3 direction = result->ray.direction.refract(result->normal, 1.0f, 1.333f);
		Ray refractionRay(origin, direction);

		CollisionResult refrResult;

		if (intersect(refractionRay, &refrResult, n + 1, 0.0f)) // TODO fog dist
			return shade(&refrResult, n + 1);

		return Color(0.0f, 0.0f, 0.0f);
	}*/

	/*Color shadeTransparency(Surface *surface, CollisionResult *result, int n) {
		float transparencyAmount = surface->getRefractionAmt();

		if (scene->settings->numTransparentRays == 0 || transparencyAmount == 0.0f)
			return Color(0.0f, 0.0f, 0.0f);

		transparencyAmount *= (1.0f / scene->settings->numTransparentRays);

		Vec3 origin = result->position + result->ray.direction * scene->settings->bias;
		Ray transparencyRay(origin, result->ray.direction);

		CollisionResult transResult;
		
		if (intersect(transparencyRay, &transResult, n + 1, 0.0f)) // TODO fog dist
			return shade(&transResult, n + 1);

		return Color(0.0f, 0.0f, 0.0f);
	}*/

	/*float shadeShadow(Surface *surface, CollisionResult *result, int n) {
		if (scene->settings->numShadowRays == 0)
			return 1.0f;

		vector<Light *>::iterator it;

		Vec3 origin = result->position + result->normal * scene->settings->bias;

		int numHit  = 0;
		int numCast = 0;

		for (it = scene->lights.begin(); it != scene->lights.end(); it++) {
			if (!(*it)->castsShadows())
				continue;

			for (int i = 0; i < scene->settings->numShadowRays; i++) {
				Vec3 direction = (*it)->getShadowDir(origin, scene->settings->numShadowRays);
				Ray shadowRay(origin, direction);

				numCast++;

				if (intersect(shadowRay, n + 1, 0.0f)) // TODO dist to light
					numHit++;
			}
		}

		if (numCast == 0)
			return 1.0f;

		float amt = 1.0f - (float)numHit / (float)numCast;
		amt = amt * 0.7f + 0.3f;

		return amt;
	}*/

	/*float shadeOcclusion(Surface *surface, CollisionResult *result, int n) {
		if (scene->settings->numOcclusionRays == 0)
			return 1.0f;

		Vec3 origin = result->position + result->normal * scene->settings->bias;

		int numHit  = 0;
		int numCast = 0;

		for (numCast = 0; numCast < scene->settings->numOcclusionRays; numCast++) {
			Vec3 direction = randHemisphere(result->normal);
			Ray occlusionRay(origin, direction);

			if (intersect(occlusionRay, n + 1, 40.0f))
				numHit++;
		}

		float amt = 1.0f - (float)numHit / (float)numCast;
		amt = pow(amt, 1.3f);
		amt = amt * 0.8f + 0.2f;

		return amt;
	}*/

	/*Color shadeSurface(Surface *surface, CollisionResult *result) {
		if (scene->settings->numShadowRays == 0)
			return 1.0f;

		vector<Light *>::iterator it;

		Color color(0.0f, 0.0f, 0.0f);

		for (it = scene->lights.begin(); it != scene->lights.end(); it++)
			color = color + surface->shade(*it, result);

		return color;
	}*/

	/*Color shadeFog(const Color & c0, const Color & c1, float fogStart, float fogEnd, float dist) {
		Color s = c0; // TODO
		Color e = c1; // TODO

		float amt = (dist - fogStart) / (fogEnd - fogStart);
		amt = CLAMP(0.0f, 1.0f, amt);
		float invAmt = 1.0f - amt;

		return s * invAmt + e * amt;
	}*/

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
		color += material->getAmbient();

		for (auto it = scene->lights.begin(); it != scene->lights.end(); it++)
			color += material->shade(*it, result);

		color.x = SATURATE(color.x);
		color.y = SATURATE(color.y);
		color.z = SATURATE(color.z);

		//vector<Light *>::iterator it;
		//Surface *surf = scene->surfaceMap[(Shape *)result->shape];

		//Color refl, refr;
		
		//Color reflection   = shadeReflection  (surf, result, n);
		//Color refraction   = shadeRefraction  (surf, result, n);
		//Color transparency = shadeTransparency(surf, result, n);
		//float shadow       = shadeShadow      (surf, result, n);
		//float occlusion    = shadeOcclusion   (surf, result, n);

		//Color surface      = shadeSurface(surf, result);

		//Color color = (surface + reflection + refraction + transparency) * shadow * occlusion;
		//color = shadeFog(color, scene->settings->fogColor, scene->settings->fogStart, scene->settings->fogEnd, result->distance);

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

			float sampleOffset = 1.0f / (float)(NSAMPLES + 1);
			float sampleContrib = 1.0f / (float)(NSAMPLES * NSAMPLES);

			for (int y = y0; y < y0 + BLOCKSZ; y++) {
				for (int x = x0; x < x0 + BLOCKSZ; x++) {
					if (x >= width || y >= height)
						continue;

					Vec3 color = Vec3(0.0f, 0.0f, 0.0f);

					for (int p = 0; p < NSAMPLES; p++) {
						float v = (float)(y + p * sampleOffset) / (float)height;

						for (int q = 0; q < NSAMPLES; q++) {
							float u = (float)(x + q * sampleOffset) / (float)width;

							Ray r = scene->camera->getViewRay(u, v);

							CollisionResult result, temp;

							/*for (auto it = scene->shapes.begin(); it != scene->shapes.end(); it++)
								if ((*it)->intersects(r, &temp) && (result.shape == NULL ||
									result.distance > temp.distance))
									result = temp;*/

							if (!tree->intersect(r, &result, 0.0f))
								continue;

							//if (result.shape != NULL)
							color += shade(&result, 1) * sampleContrib;
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

		printf("Done: %f seconds\n", timer.getElapsedMilliseconds() / 1000.0);

		if (display != NULL) {
			display->refresh();
			printf("Press any key to exit: ");
			getchar();
		}
	}
};

#endif