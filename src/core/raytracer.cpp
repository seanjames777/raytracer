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

		const util::vector<Triangle, 16> & triangles = scene->getTriangles();

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
	KDPacketStackFrame<4> stack[64]; // TODO

	struct RadianceRayStackFrame {
		int2 pixel;
		float3 weight;
	};

	struct ShadowRayStackFrame {
		Ray ray;
		int2 pixel;
		float3 weight;
		float maxDist;
	};

	struct RadianceRayStack {
		util::vector<RadianceRayStackFrame, 16> frame;
		util::vector<float, 16> origin[3];
		util::vector<float, 16> direction[3];
	};

	RadianceRayStack radianceStack[8];

	util::vector<ShadowRayStackFrame, 16> shadowStack;

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
						frame.pixel = int2(x, y);
						frame.weight = weight;

						int c = (signbit(r.direction.x) << 2) | (signbit(r.direction.y) << 1) | (signbit(r.direction.z) << 0);

						radianceStack[c].frame.push_back(frame);

						radianceStack[c].origin[0].push_back(r.origin.x);
						radianceStack[c].origin[1].push_back(r.origin.y);
						radianceStack[c].origin[2].push_back(r.origin.z);

						radianceStack[c].direction[0].push_back(r.direction.x);
						radianceStack[c].direction[1].push_back(r.direction.y);
						radianceStack[c].direction[2].push_back(r.direction.z);
					}
				}
			}
		}

		struct ShadingWorkItem {
			Ray ray;
			RadianceRayStackFrame frame;
			Collision collision; // TODO: mess with offset of triangle ID
		};

		util::vector<ShadingWorkItem, 16> shadingBuff;

		// Alternate between tree traversal and shading. Shading may produce more traversal work.
		for (int generation = 0; generation < settings.maxDepth; generation++) {
			// Note: rays now have same sign bits in each direction

			for (int i = 0; i < 8; i++)
				for (int j = 0; j < (radianceStack[i].frame.size() & ~(SIMD - 1)); j += SIMD) { // TODO: handle last elements
					PacketCollision<SIMD> result;
					Packet<SIMD> packet;

					// TODO: Avoid this copy by passing registers directly
					packet.origin[0] = *(vector<float, SIMD> *)&radianceStack[i].origin[0][j];
					packet.origin[1] = *(vector<float, SIMD> *)&radianceStack[i].origin[1][j];
					packet.origin[2] = *(vector<float, SIMD> *)&radianceStack[i].origin[2][j];

					packet.direction[0] = *(vector<float, SIMD> *)&radianceStack[i].direction[0][j];
					packet.direction[1] = *(vector<float, SIMD> *)&radianceStack[i].direction[1][j];
					packet.direction[2] = *(vector<float, SIMD> *)&radianceStack[i].direction[2][j];

					vector<bmask, SIMD> hit = tree->intersectPacket((KDPacketStackFrame<SIMD> *)stack, // TODO fix cast
						packet, vector<float, SIMD>(INFINITY), result);

					// TODO: Might be useful to pass in active mask

					for (int k = 0; k < SIMD; k++) {
						if (hit[k]) {
							ShadingWorkItem item;

							item.ray.origin[0] = radianceStack[i].origin[0][j + k];
							item.ray.origin[1] = radianceStack[i].origin[1][j + k];
							item.ray.origin[2] = radianceStack[i].origin[2][j + k];

							item.ray.direction[0] = radianceStack[i].direction[0][j + k];
							item.ray.direction[1] = radianceStack[i].direction[1][j + k];
							item.ray.direction[2] = radianceStack[i].direction[2][j + k];

							item.frame = radianceStack[i].frame[j + k];

							item.collision.beta = result.beta[k];
							item.collision.gamma = result.gamma[k];
							item.collision.distance = result.distance[k];
							item.collision.triangle_id = result.triangle_id[k];

							shadingBuff.push_back(item);
						}
					}
				}

			for (int i = 0; i < 8; i++) {
				radianceStack[i].frame.clear();

				radianceStack[i].origin[0].clear();
				radianceStack[i].origin[1].clear();
				radianceStack[i].origin[2].clear();

				radianceStack[i].direction[0].clear();
				radianceStack[i].direction[1].clear();
				radianceStack[i].direction[2].clear();
			}

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

				Image<float, 4> *normalMap = material->getNormalTexture();

				if (normalMap) {
					float3 tangent = normalize(interp.tangent);
					float3 bitangent = cross(interp.normal, tangent);

					Sampler sampler(Bilinear, Wrap);
					float3 tbn = sampler.sample(normalMap, interp.uv).xyz() * 2.0f - 1.0f;

					// TODO: extra normalize might not be needed
					interp.normal = normalize(tbn.x * tangent - tbn.y * bitangent + tbn.z * interp.normal);
				}

				float3 wo = -item.ray.direction;

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

				if (generation < settings.maxDepth - 1) {
					Ray indirectRay;
					RadianceRayStackFrame indirect;

					indirectRay.origin = interp.position + triangle->normal * 0.001f;

					float secondary;
					rand1D(1, &secondary);

					if (secondary < 0.5f || material->getReflectivity() == 0.0f) {
						float2 rand;
						rand2D(1, &rand);

						mapSamplesCosHemisphere(1, 1.0f, &rand, &indirectRay.direction);
						alignHemisphereNormal(1, &indirectRay.direction, triangle->normal);

						// Importance sampling: n dot l term cancels out
						indirect.weight = item.frame.weight * material->f(interp, wo, indirectRay.direction) * (float)M_PI;
					}
					else {
						indirectRay.direction = reflect(wo, interp.normal);
						indirect.weight = item.frame.weight * material->getReflectivity(); // TODO: hack
						//float ndotl = saturate(dot(r.direction, interp.normal));
						//weight *= material->f(interp, wo, r.direction) * ndotl;
					}

					indirect.pixel = item.frame.pixel;

					int c = (signbit(indirectRay.direction.x) << 2) | (signbit(indirectRay.direction.y) << 1) | (signbit(indirectRay.direction.z) << 0);

					radianceStack[c].frame.push_back(indirect);

					radianceStack[c].origin[0].push_back(indirectRay.origin.x);
					radianceStack[c].origin[1].push_back(indirectRay.origin.y);
					radianceStack[c].origin[2].push_back(indirectRay.origin.z);

					radianceStack[c].direction[0].push_back(indirectRay.direction.x);
					radianceStack[c].direction[1].push_back(indirectRay.direction.y);
					radianceStack[c].direction[2].push_back(indirectRay.direction.z);
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

			if (!tree->intersect((KDStackFrame *)stack, r, frame.maxDist, result)) { // TODO fix
				// TODO
				float3 color = scene->getOutput()->getPixel(frame.pixel.x, frame.pixel.y).xyz();
				color = color + frame.weight; // TODO
				scene->getOutput()->setPixel(frame.pixel.x, frame.pixel.y, float4(color, 1.0f));
			}
		}

		shadowStack.clear();

        // TODO: Flushing one tile at a time keeps the tile in the cache probably, but might
        // not get the most coherence. Adjusting the tile size would affect this probably.
    }

    //std::cout << "Ray buffer size: " << rayBuff.capacity() << " (" << (rayBuff.capacity() * sizeof(Ray) + 1024 - 1) / 1024 << "kb)" << std::endl;

    numThreadsAlive--;
}
