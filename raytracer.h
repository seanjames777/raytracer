/*
 * Sean James
 *
 * raytracer.h
 *
 * Raytracer
 *
 */

#ifndef _RAYTRACER_H
#define _RAYTRACER_H

#include "shape.h"
#include "defs.h"
#include "rtmath.h"
#include "camera.h"
#include "bitmap.h"
#include "light.h"
#include "surface.h"
#include "timer.h"
#include "glimagedisplay.h"
#include "thread.h"
#include "atomicint.h"
#include "threadpool.h"
#include "kdtree.h"
#include "loggers.h"

#include <map>
#include <ctime>

using namespace std;

/*
 * Object containing raytracer configuration
 */
struct RaytracerSettings {
	int maxBounces;          // Maximum number of ray bounces
	int numCameraRays;       // Number of depth-of-field rays
	int numShadowRays;       // Number of soft-shadow rays
	int numTransparentRays;  // Number of foggy transparency rays
	int numReflectionRays;   // Number of glossy reflection rays
	int numRefractionRays;   // Number of glossy refraction rays
	int numOcclusionRays;    // Number of ambient occlusion rays
	int numSamples;          // sqrt(Number of samples per pixel)
	float bias;              // Ray offset for bounce rays
	Color fogColor;          // Fog color
	float fogStart;          // Distance from camera where fog is transparent
	float fogEnd;            // Distance from camera where fog is opaque

	/* 
	 * Empty constructor sets default settings
	 */
	RaytracerSettings()
		: maxBounces(3),
		  numCameraRays(1),
		  numShadowRays(1),
		  numTransparentRays(0),
		  numReflectionRays(0),
		  numRefractionRays(0),
		  numOcclusionRays(0),
		  numSamples(1),
		  bias(0.001f),
		  fogColor(Color(0.435f, 0.6f, 0.7f)),
		  fogStart(1500.0f),
		  fogEnd(1800.0f)
	{
	}
};

class Raytracer {
private:

	KDTree *tree;
	RaytracerSettings settings;

	bool intersect(Ray & ray, int n, float maxDepth) {
		if (n > settings.maxBounces)
			return false;

		return tree->intersect(ray, maxDepth);
	}

	bool intersect(Ray & ray, CollisionResult *result, int n, float maxDepth) {
		if (n > settings.maxBounces)
			return false;

		return tree->intersect(ray, result, maxDepth);
	}

	Color shadeReflection(Surface *surface, CollisionResult *result, int n) {
		float reflectionAmount = surface->getReflectionAmt();

		if (settings.numReflectionRays == 0 || reflectionAmount == 0.0f)
			return Color(0.0f, 0.0f, 0.0f);

		reflectionAmount *= (1.0f / settings.numReflectionRays);

		Vec3 origin = result->position + result->normal * settings.bias;
		Vec3 direction = result->ray.direction.reflect(result->normal);
		Ray reflectionRay(origin, direction);

		CollisionResult reflResult;

		if (intersect(reflectionRay, &reflResult, n + 1, 0.0f)) // TODO fog dist
			return shade(&reflResult, n + 1);

		return Color(0.0f, 0.0f, 0.0f);
	}

	Color shadeRefraction(Surface *surface, CollisionResult *result, int n) {
		float refractionAmount = surface->getRefractionAmt();

		if (settings.numRefractionRays == 0 || refractionAmount == 0.0f)
			return Color(0.0f, 0.0f, 0.0f);

		refractionAmount *= (1.0f / settings.numRefractionRays);

		Vec3 origin = result->position + result->ray.direction * settings.bias;
		Vec3 direction = result->ray.direction.refract(result->normal, 1.0f, 1.333f);
		Ray refractionRay(origin, direction);

		CollisionResult refrResult;

		if (intersect(refractionRay, &refrResult, n + 1, 0.0f)) // TODO fog dist
			return shade(&refrResult, n + 1);

		return Color(0.0f, 0.0f, 0.0f);
	}

	Color shadeTransparency(Surface *surface, CollisionResult *result, int n) {
		float transparencyAmount = surface->getRefractionAmt();

		if (settings.numTransparentRays == 0 || transparencyAmount == 0.0f)
			return Color(0.0f, 0.0f, 0.0f);

		transparencyAmount *= (1.0f / settings.numTransparentRays);

		Vec3 origin = result->position + result->ray.direction * settings.bias;
		Ray transparencyRay(origin, result->ray.direction);

		CollisionResult transResult;
		
		if (intersect(transparencyRay, &transResult, n + 1, 0.0f)) // TODO fog dist
			return shade(&transResult, n + 1);

		return Color(0.0f, 0.0f, 0.0f);
	}

	float shadeShadow(Surface *surface, CollisionResult *result, int n) {
		if (settings.numShadowRays == 0)
			return 1.0f;

		vector<Light *>::iterator it;

		Vec3 origin = result->position + result->normal * settings.bias;

		int numHit  = 0;
		int numCast = 0;

		for (it = lights.begin(); it != lights.end(); it++) {
			if (!(*it)->castsShadows())
				continue;

			for (int i = 0; i < settings.numShadowRays; i++) {
				Vec3 direction = (*it)->getShadowDir(origin, settings.numShadowRays);
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
	}

	float shadeOcclusion(Surface *surface, CollisionResult *result, int n) {
		if (settings.numOcclusionRays == 0)
			return 1.0f;

		Vec3 origin = result->position + result->normal * settings.bias;

		int numHit  = 0;
		int numCast = 0;

		for (numCast = 0; numCast < settings.numOcclusionRays; numCast++) {
			Vec3 direction = randHemisphere(result->normal);
			Ray occlusionRay(origin, direction);

			if (intersect(occlusionRay, n + 1, 40.0f))
				numHit++;
		}

		float amt = 1.0f - (float)numHit / (float)numCast;
		amt = pow(amt, 1.3f);
		amt = amt * 0.8f + 0.2f;

		return amt;
	}

	Color shadeSurface(Surface *surface, CollisionResult *result) {
		if (settings.numShadowRays == 0)
			return 1.0f;

		vector<Light *>::iterator it;

		Color color(0.0f, 0.0f, 0.0f);

		for (it = lights.begin(); it != lights.end(); it++)
			color = color + surface->shade(*it, result);

		return color;
	}

	Color shadeFog(const Color & c0, const Color & c1, float fogStart, float fogEnd, float dist) {
		Color s = c0; // TODO
		Color e = c1; // TODO

		float amt = (dist - fogStart) / (fogEnd - fogStart);
		amt = CLAMP(0.0f, 1.0f, amt);
		float invAmt = 1.0f - amt;

		return s * invAmt + e * amt;
	}

	Color shade(CollisionResult *result, int n) {
		vector<Light *>::iterator it;
		Surface *surf = surfaces.at((Shape *)result->shape);

		Color refl, refr;
		
		Color reflection   = shadeReflection  (surf, result, n);
		Color refraction   = shadeRefraction  (surf, result, n);
		Color transparency = shadeTransparency(surf, result, n);
		float shadow       = shadeShadow      (surf, result, n);
		float occlusion    = shadeOcclusion   (surf, result, n);

		Color surface      = shadeSurface(surf, result);

		Color color = (surface + reflection + refraction + transparency) * shadow * occlusion;
		color = shadeFog(color, settings.fogColor, settings.fogStart, settings.fogEnd, result->distance);

		return color;
	}

	vector<Shape *> shapes;
	vector<Light *> lights;
	map<Shape *, Surface *> surfaces;
	Camera *camera;
	Bitmap *output;

	int width, height;
	float aperture, focus;

	AtomicInt blocksDone;

public:

	Raytracer() 
		: camera(NULL), output(NULL), blocksDone(0)
	{
		srand((unsigned)time(0));
	}

	void setCamera(Camera *camera, float aperture, float focus) {
		this->camera = camera;
		this->aperture = aperture;
		this->focus = focus;
	}

	void addShape(Shape *shape) {
		shapes.push_back(shape);
	}

	void addLight(Light *light) {
		lights.push_back(light);
	}

	void setSurface(Shape *shape, Surface *surface) {
		surfaces[shape] = surface;
	}

	void setOutput(Bitmap *bitmap) {
		output = bitmap;
		width = bitmap->getWidth();
		height = bitmap->getHeight();
	}

	struct BlockArgs {
		int2 blockDim, blockIdx;
		Raytracer *rt;

		BlockArgs()
			: blockDim(0, 0), blockIdx(0, 0), rt(NULL)
		{
		}
	};

	Color colors[100];

	static void *renderBlock(void *arg) {
		BlockArgs *args = (BlockArgs *)arg;
		Raytracer *rt = args->rt;

		int x0 = args->blockDim.x * args->blockIdx.x;
		int y0 = args->blockDim.y * args->blockIdx.y;

		if (x0 > rt->width || y0 > rt->height) {
			rt->blocksDone.postIncrement();
			return NULL;
		}

		int blockWidth  = rt->width  - x0 > args->blockDim.x ? args->blockDim.x : rt->width  - x0;
		int blockHeight = rt->height - y0 > args->blockDim.y ? args->blockDim.y : rt->height - y0;

		float sampleOffset = 1.0f / (float)(rt->settings.numSamples + 1);
		float sampleContribution = 1.0f / (rt->settings.numSamples * rt->settings.numSamples);
		float camRayContribution = 1.0f / (float)rt->settings.numCameraRays;

		for (int y = y0; y < y0 + blockHeight; y++) {
			for (int x = x0; x < x0 + blockWidth; x++) {

				Color color = Color(0.0f, 0.0f, 0.0f);

				for (int p = 0; p < rt->settings.numSamples; p++) {
					float v = (float)(y + p * sampleOffset) / (rt->height);

					for (int q = 0; q < rt->settings.numSamples; q++) {
						float u = (float)(x + q * sampleOffset) / (rt->width);

						bool hasHit = false;
						Color contrib = rt->settings.fogColor;

						for (int c = 0; c < rt->settings.numCameraRays; c++) {
							Ray r = rt->camera->getViewRay(u, v, rt->focus, rt->aperture, rt->settings.numCameraRays);

							CollisionResult result;

							if (rt->intersect(r, &result, 1, 0.0f)) { // TODO fog dist
								Color me = rt->shade(&result, 1) * camRayContribution;

								if (!hasHit) {
									contrib = me;
									hasHit = true;
								}
								else
									contrib = contrib + me;
							}
						}

						color = color + contrib * sampleContribution;
					}
				}

				//color = Color(1.0f, 0.0f, 0.0f);

				rt->output->setPixel(x, y, color);
			}
		}

		rt->blocksDone.postIncrement();

		return 0;
	}

	void schedule(ThreadPool *pool, int x, int y, int currSize, int n, BlockArgs *args) {
		if (currSize == 1) {
			pool->submit(&renderBlock, &args[y * n + x]);
		}
		else {
			currSize /= 2;

			schedule(pool, x           , y           , currSize, n, args);
			schedule(pool, x + currSize, y           , currSize, n, args);
			schedule(pool, x           , y + currSize, currSize, n, args);
			schedule(pool, x + currSize, y + currSize, currSize, n, args);
		}
	}

	int pow2Above(int x) {
		int y = 1;

		while (y < x)
			y <<= 1;

		return y;
	}

	AABB buildSceneAABB() {
		AABB box = shapes[0]->getBBox();

		vector<Shape *>::iterator it;
		for (it = shapes.begin(); it != shapes.end(); it++) {
			AABB myBox = (*it)->getBBox();
			box.join(myBox);
		}

		return box;
	}

	void render(int blockSize, GLImageDisplay *display) {
		if (camera == NULL || output == NULL) {
			printf("ERROR camera or output NULL\n");
			return;
		}

		for (int i = 0; i < 100; i++)
			colors[i] = Color(randf(0.5f, 1.0f), randf(0.5f, 1.0f), randf(0.5f, 1.0f));

		vector<KDContainable *> asContainable;

		vector<Shape *>::iterator it;
		for (it = shapes.begin(); it != shapes.end(); it++)
			asContainable.push_back((KDContainable *)*it);

		Timer timer;
		timer.start();

		blocksDone.set(0);

		int nBlocksW = width  / blockSize + (width  % blockSize == 0 ? 0 : 1);
		int nBlocksH = height / blockSize + (height % blockSize == 0 ? 0 : 1);
		int sx = pow2Above(nBlocksW);
		int sy = pow2Above(nBlocksH);
		int n  = MAX2(sx, sy);

		ThreadPool pool(7, n * n, false);
		tree = new KDTree(asContainable, buildSceneAABB(), pool.getNumThreads());

		BlockArgs *argsPool = (BlockArgs *)malloc(sizeof(BlockArgs) * n * n);

		for (int y = 0; y < n; y++) {
			for (int x = 0; x < n; x++) {
				BlockArgs args;
				args.blockDim.x = blockSize;
				args.blockDim.y = blockSize;
				args.blockIdx.x = x;
				args.blockIdx.y = y;
				args.rt = this;

				argsPool[y * n + x] = args;
			}
		}

		schedule(&pool, 0, 0, n, n, argsPool);
		pool.start();

		while (true) {
			float p = (float)blocksDone.get() / (float)(n * n) * 100.0f;
			printf("%.2f%%\n", p);

			if (display != NULL)
				display->refresh();

			if (blocksDone.get() == n * n)
				break;
		}

		//pool.kill();
		display->refresh();
		free(argsPool);

		Loggers::Main.Log(LOGGER_CRITICAL, "Render DONE\n");
		timer.stopAndPrint("Render");

		tree->printDebugInfo();
	}
};

#endif