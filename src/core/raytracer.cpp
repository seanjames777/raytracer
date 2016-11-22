/**
 * @file core/raytracer.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/raytracer.h>

#include <iostream>

// TODO: Increasing tile size seems to make the computer happy
#define BLOCKW 32
#define BLOCKH 32

Raytracer::Raytracer(RaytracerSettings settings, Scene *scene)
    : settings(settings),
      scene(scene),
      tree(nullptr)
{
    srand((unsigned)time(0));
}

Raytracer::~Raytracer() {
    if(tree)
        delete tree;
}

void Raytracer::render() {
    shouldShutdown = false;

    if (!tree) {
        KDSAHBuilder builder;

        // TODO
		void *nodeMem = aligned_alloc(1024 * 1024 * 100, 8);
		void *triangleMem = aligned_alloc(1024 * 1024 * 100, 8);

		KDAllocator nodeAllocator(nodeMem, 1024 * 1024 * 100);
		KDAllocator triangleAllocator(triangleMem, 1024 * 1024 * 100);

		const std::vector<Triangle> & triangles = scene->getTriangles();

		tree = builder.build(&triangles[0], triangles.size(), &nodeAllocator, &triangleAllocator, &stats);
    }

    nBlocksW = (scene->getOutput()->getWidth() + BLOCKW - 1) / BLOCKW;
    nBlocksH = (scene->getOutput()->getHeight() + BLOCKH - 1) / BLOCKH;
    nBlocks = nBlocksW * nBlocksH;

    currBlockID = 0;

    int nThreads = settings.numThreads;

    if (nThreads == 0)
        nThreads = std::thread::hardware_concurrency(); // TODO: Maybe better way to update image

    numThreadsAlive = nThreads;

    for (int i = 0; i < nThreads; i++)
        workers.push_back(std::make_shared<std::thread>(std::bind(&Raytracer::worker_thread,
            this, i, nThreads)));

    printf("Started %d worker threads\n", nThreads);
}

void Raytracer::shutdown(bool waitUntilFinished) {
    if (!waitUntilFinished)
        shouldShutdown = true;

    for (auto& worker : workers)
        worker->join();

    workers.clear();
}

void Raytracer::worker_thread(int idx, int numThreads) {
    int width = scene->getOutput()->getWidth();
    int height = scene->getOutput()->getHeight();

    float2 invImageSize = float2(1.0f / (float)width, 1.0f / (float)height);
    float sampleContrib = 1.0f / (float)(settings.pixelSamples * settings.pixelSamples);

    int blockID = idx;

	assert(stats.max_depth < 64);
	KDStackFrame stack[64]; // TODO

	struct RadianceRayStackFrame {
		Ray ray;
		uint2 pixel;
		float3 weight;
		char depth;
	};

	struct ShadowRayStackFrame {
		Ray ray;
		uint2 pixel;
		float3 weight;
		float maxDist;
	};

#define DIRSTACKS 0

#if DIRSTACKS
	std::vector<RadianceRayStackFrame> radianceStack[8];
#else
	std::vector<RadianceRayStackFrame> radianceStack;
#endif
	std::vector<ShadowRayStackFrame> shadowStack;

	float3 min = tree->bounds().min;
	float3 max = tree->bounds().max;
	float3 minMaxDenom = 1.0f / (max - min);

    while(!shouldShutdown) {
        blockID = currBlockID++;

        if (blockID >= nBlocks)
            break;

        int y = blockID / nBlocksW;
        int x = blockID % nBlocksW;

        int x0 = BLOCKW * x;
        int y0 = BLOCKH * y;

		for (int y = y0; y < y0 + BLOCKH; y++) {
			for (int x = x0; x < x0 + BLOCKW; x++) {
				if (x >= width || y >= height)
					continue;

				float3 color(0.0f);

				// Take jittered sampled to reduce variance and move from stairstepping
				// artifacts to noise
				#define MAX_PIXEL_SAMPLES 16
				float2 samples[MAX_PIXEL_SAMPLES * MAX_PIXEL_SAMPLES];
				randJittered2D(settings.pixelSamples, samples);

				// TODO: Is the pointer chasing through scene bad?

				// TODO: It's possible to do better sampling

				float2 xy = float2(x, y);

				for (int p = 0; p < settings.pixelSamples; p++) {
					for (int q = 0; q < settings.pixelSamples; q++) {
						float2 uv = (xy + samples[p * settings.pixelSamples + q]) * invImageSize;

						Ray r = scene->getCamera()->getViewRay(uv);

						// float3 sampleColor = getEnvironment(r.direction); TODO
						float3 weight(1.0f / (settings.pixelSamples * settings.pixelSamples));

						RadianceRayStackFrame frame;
						frame.ray = r;
						frame.pixel = uint2(x, y);
						frame.weight = weight;
						frame.depth = 0;

#if DIRSTACKS
						int c = (signbit(r.direction.x) << 2) | (signbit(r.direction.y) << 1) | (signbit(r.direction.z) << 0);
						radianceStack[c].push_back(frame);
#else
						radianceStack.push_back(frame);
#endif
					}
				}
			}
		}

		struct ShadingWorkItem {
			RadianceRayStackFrame frame;
			Collision collision; // TODO: mess with offset of triangle ID
		};

		std::vector<ShadingWorkItem> shadingBuff;

		// Alternate between tree traversal and shading. Shading may produce more traversal work.
#if DIRSTACKS
		while (!radianceStack[0].empty() || !radianceStack[1].empty() || !radianceStack[2].empty() || !radianceStack[3].empty() || !radianceStack[4].empty() || !radianceStack[5].empty() || !radianceStack[6].empty() || !radianceStack[7].empty()) {
#else
		while (!radianceStack.empty()) {
#endif

#if !DIRSTACKS && 0
			// TODO: Bucket instead of sort
			std::sort(radianceStack.begin(), radianceStack.end(), [&](const RadianceRayStackFrame & l, const RadianceRayStackFrame & r) {
#if 0
				float3 pl = saturate((l.ray.origin - min) * minMaxDenom) * (float)((1 << 10) - 1); // TODO: precompute
				uint3 il = uint3(pl.x, pl.y, pl.z); // TODO: cast function

				float3 pr = saturate((r.ray.origin - min) * minMaxDenom) * (float)((1 << 10) - 1); // TODO: precompute
				uint3 ir = uint3(pr.x, pr.y, pr.z); // TODO: cast function
#else
				uint3 il(0);
				uint3 ir(0);
#endif

				// TODO: Using 9 bits per component, but we could trade off vertical bits for horizontal for most scenes, probably.

				int cl = (il.x << 22) | (il.y << 13) | (il.z << 4) | (signbit(l.ray.direction.x) << 2) | (signbit(l.ray.direction.y) << 1) | (signbit(l.ray.direction.z) << 0);
				int cr = (ir.x << 22) | (ir.y << 13) | (ir.z << 4) | (signbit(r.ray.direction.x) << 2) | (signbit(r.ray.direction.y) << 1) | (signbit(r.ray.direction.z) << 0);

				return cl < cr;
			});
#endif

			// TODO: trace rays in SIMD
			// Note: rays now have same sign bits in each direction

#if DIRSTACKS
			for (int i = 0; i < 8; i++)
			for (auto & frame : radianceStack[i]) {
#else
			for (auto & frame : radianceStack) {
#endif
				Collision result;
				Ray r = frame.ray;

				if (tree->intersect(stack, r, false, INFINITY, result)) {
					ShadingWorkItem item;
					item.frame = frame;
					item.collision = result;

					shadingBuff.push_back(item);
				}
			}

#if DIRSTACKS
			for (int i = 0; i < 8; i++)
			radianceStack[i].clear();
#else
			radianceStack.clear();
#endif

#if 1
			// TODO: Better sorting is probably possible
			// Ideas: * Insert into buckets or sorted array or hash table
			//        * Bitonic sort
			//        * SIMD sort?
			//        * Shade in SIMD since we're coherent now
			//        * Include material ID in sort
			std::sort(shadingBuff.begin(), shadingBuff.end(), [](const ShadingWorkItem & l, const ShadingWorkItem & r) -> bool {
				return l.collision.triangle_id < r.collision.triangle_id;
			});
#endif

			for (auto & item : shadingBuff) {
				const Triangle *triangle = scene->getTriangle(item.collision.triangle_id);

				// TODO: Significant cache miss here pulling vertex data in from memory. Try sorting shading work by
				// triangle?
				Vertex interp = triangle->interpolate(item.collision.beta, item.collision.gamma);
				interp.normal = normalize(interp.normal); // TODO: do we want to do this here?

				const Material *material = scene->getMaterial(triangle->material_id);

				Image<float, 3> *normalMap = material->getNormalTexture();

				if (normalMap) {
					float3 tangent = normalize(interp.tangent);
					float3 bitangent = cross(interp.normal, tangent);

					Sampler sampler(Bilinear, Wrap);
					float3 tbn = sampler.sample(normalMap, interp.uv) * 2.0f - 1.0f;

					// TODO: extra normalize might not be needed
					interp.normal = normalize(tbn.x * tangent - tbn.y * bitangent + tbn.z * interp.normal);
				}

				float3 wo = -item.frame.ray.direction;

				for (int l = 0; l < scene->getNumLights(); l++) {
					const Light *light = scene->getLight(l);

					float3 wi, Li;
					float r;
					light->sample(interp.position, wi, r, Li);

					float ndotl = saturate(dot(wi, interp.normal)); // TODO: normal mapping

					ShadowRayStackFrame shadow;
					shadow.ray = Ray(interp.position + triangle->normal * 0.001f, wi);
					shadow.pixel = item.frame.pixel;
					shadow.weight = item.frame.weight * Li * material->f(interp, wo, wi) * ndotl;
					shadow.maxDist = r;

					// TODO: If light does not cast shadows, return color immediately

					shadowStack.push_back(shadow);
				}

				if (item.frame.depth < settings.maxDepth - 1) {
					RadianceRayStackFrame indirect;

					indirect.ray.origin = interp.position + triangle->normal * 0.001f;

					float secondary;
					rand1D(1, &secondary);

					if (secondary < 0.5f || material->getReflectivity() == 0.0f) {
						float2 rand;
						rand2D(1, &rand);

						mapSamplesCosHemisphere(1, 1.0f, &rand, &indirect.ray.direction);
						alignHemisphereNormal(1, &indirect.ray.direction, triangle->normal);

						// Importance sampling: n dot l term cancels out
						indirect.weight = item.frame.weight * material->f(interp, wo, indirect.ray.direction) * (float)M_PI;
					}
					else {
						indirect.ray.direction = reflect(wo, interp.normal);
						indirect.weight = item.frame.weight * material->getReflectivity(); // TODO: hack
						//float ndotl = saturate(dot(r.direction, interp.normal));
						//weight *= material->f(interp, wo, r.direction) * ndotl;
					}

					indirect.pixel = item.frame.pixel;
					indirect.depth = item.frame.depth + 1;

#if DIRSTACKS
					int c = (signbit(indirect.ray.direction.x) << 2) | (signbit(indirect.ray.direction.y) << 1) | (signbit(indirect.ray.direction.z) << 0);
					radianceStack[c].push_back(indirect);
#else
					radianceStack.push_back(indirect);
#endif
				}
			}

			shadingBuff.clear();
		}

#if 0
		// TODO: Bucket instead of sort
		std::sort(shadowStack.begin(), shadowStack.end(), [](const ShadowRayStackFrame & l, const ShadowRayStackFrame & r) {
			int cl = (signbit(l.ray.direction.x) << 2) | (signbit(l.ray.direction.y) << 1) | (signbit(l.ray.direction.z) << 0);
			int cr = (signbit(r.ray.direction.x) << 2) | (signbit(r.ray.direction.y) << 1) | (signbit(r.ray.direction.z) << 0);

			return cl < cr;
		});
#endif

		// Shadow rays do not generate new rays, so we only need to iterate once
		for (auto & frame : shadowStack) {
			Collision result;
			Ray r = frame.ray;

			if (!tree->intersect(stack, r, true, frame.maxDist, result)) {
				// TODO
				float3 color = scene->getOutput()->getPixel(frame.pixel.x, frame.pixel.y);
				color += frame.weight; // TODO
				scene->getOutput()->setPixel(frame.pixel.x, frame.pixel.y, color);
			}
		}

		shadowStack.clear();

        // TODO: Flushing one tile at a time keeps the tile in the cache probably, but might
        // not get the most coherence. Adjusting the tile size would affect this probably.
    }

    //std::cout << "Ray buffer size: " << rayBuff.capacity() << " (" << (rayBuff.capacity() * sizeof(Ray) + 1024 - 1) / 1024 << "kb)" << std::endl;

    numThreadsAlive--;
}
