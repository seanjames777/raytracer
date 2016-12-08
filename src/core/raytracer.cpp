/**
 * @file core/raytracer.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/raytracer.h>

#include <iostream>
#include <cassert>

// TODO: thread affinity

// TODO: Increasing tile size seems to make the computer happy
#define BLOCKW 32
#define BLOCKH 32

Raytracer::Raytracer(RaytracerSettings settings, Scene *scene, Image<float, 4> *output)
    : settings(settings),
      scene(scene),
      output(output)
{
    // TODO: make these runtime errors
    assert(scene->getCamera());

    scene->getCamera()->setAspectRatio((float)output->getWidth() / (float)output->getHeight());

    srand((unsigned)time(0));
}

Raytracer::~Raytracer() {
}

void Raytracer::render() {
    shouldShutdown = false;

    KDSAHBuilder builder(tree, scene->getTriangles(), 12.0f, 1.0f);
    builder.build(&_treeStats);

    nBlocksW = (output->getWidth() + BLOCKW - 1) / BLOCKW;
    nBlocksH = (output->getHeight() + BLOCKH - 1) / BLOCKH;
    nBlocks = nBlocksW * nBlocksH;

    currBlockID = 0;

    int nThreads = settings.numThreads;

    if (nThreads == 0)
        nThreads = std::thread::hardware_concurrency(); // TODO: Maybe better way to update image

    numThreadsAlive = nThreads;

	for (int i = 0; i < nThreads; i++)
		workerStats.push_back(RaytracerStats());

	for (int i = 0; i < nThreads; i++) {
		workers.push_back(std::make_shared<std::thread>(std::bind(&Raytracer::worker_thread,
			this, i, nThreads, &workerStats[i])));
	}

    printf("Started %d worker threads\n", nThreads);
}

void Raytracer::shutdown(bool waitUntilFinished, RaytracerStats *stats) {
    if (!waitUntilFinished)
        shouldShutdown = true;

    for (auto& worker : workers)
        worker->join();

	if (stats) {
		memset(stats, 0, sizeof(RaytracerStats));

		int nThreads = workers.size();

		for (int i = 0; i < nThreads; i++) {
			for (int j = 0; j < RaytracerStatCount; j++)
				stats->stat[j] += workerStats[i].stat[j];
		}

		stats->stat[RaytracerStatUnaccountedCycles] = stats->stat[RaytracerStatTotalCycles];

		for (int j = 1; j < RaytracerStatCount - 1; j++)
			stats->stat[RaytracerStatUnaccountedCycles] -= stats->stat[j];
	}

	workerStats.clear();
    workers.clear();
}

void Raytracer::worker_thread(int idx, int numThreads, RaytracerStats *stats) {
	memset(stats, 0, sizeof(RaytracerStats));

	int width = output->getWidth();
    int height = output->getHeight();

    float2 invImageSize = float2(1.0f / (float)width, 1.0f / (float)height);
    float sampleContrib = 1.0f / (float)(settings.pixelSamples * settings.pixelSamples);

    int blockID = idx;

    // TODO: the depth is bounded to 24... no need for such a big stack?
	assert(_treeStats.max_depth < 64);
	KDPacketStackFrame<4> stack[64]; // TODO

	RayStack radianceStack[8];
	RayStack shadowStack[8];

	int numRays = BLOCKW * BLOCKH * settings.pixelSamples * settings.pixelSamples;

	for (int i = 0; i < sizeof(radianceStack) / sizeof(radianceStack[0]); i++) {
		radianceStack[i].frame.reserve(numRays);

		radianceStack[i].origin[0].reserve(numRays);
		radianceStack[i].origin[1].reserve(numRays);
		radianceStack[i].origin[2].reserve(numRays);

		radianceStack[i].direction[0].reserve(numRays);
		radianceStack[i].direction[1].reserve(numRays);
		radianceStack[i].direction[2].reserve(numRays);

		// Max dist is not used
	}

	struct ShadingWorkItem {
		Ray ray;
		RayStackFrame frame;
		Collision collision; // TODO: mess with offset of triangle ID
	};

	util::vector<ShadingWorkItem, 16> shadingBuff;

	shadingBuff.reserve(numRays);

	for (int i = 0; i < sizeof(shadowStack) / sizeof(shadowStack[0]); i++) {
		shadowStack[i].frame.reserve(numRays);

		shadowStack[i].origin[0].reserve(numRays);
		shadowStack[i].origin[1].reserve(numRays);
		shadowStack[i].origin[2].reserve(numRays);

		shadowStack[i].direction[0].reserve(numRays);
		shadowStack[i].direction[1].reserve(numRays);
		shadowStack[i].direction[2].reserve(numRays);

		shadowStack[i].maxDist.reserve(numRays);
	}

	util::vector<ShadingWorkItem, 16> shadowBuff;

	shadowBuff.reserve(numRays);

    while(!shouldShutdown) {
        blockID = currBlockID++;

        if (blockID >= nBlocks)
            break;

		StatTimer totalCycles = startStatTimer(RaytracerStatTotalCycles);

        int y = blockID / nBlocksW;
        int x = blockID % nBlocksW;

        int x0 = BLOCKW * x;
        int y0 = BLOCKH * y;

		for (int y = y0; y < y0 + BLOCKH; y++) {
			for (int x = x0; x < x0 + BLOCKW; x++) {
				if (x >= width || y >= height)
					continue;

				float3 color(0.0f);

				// TODO: Is the pointer chasing through scene bad?

				// TODO: It's possible to do better sampling

				float2 xy = float2(x, y);

				for (int p = 0; p < settings.pixelSamples; p++) {
					for (int q = 0; q < settings.pixelSamples; q++) {
						StatTimer primaryEmit = startStatTimer(RaytracerStatPrimaryEmitCycles);

						// Take jittered sampled to reduce variance and move from stairstepping
						// artifacts to noise
						float2 xy2 = (xy + randJittered2D(settings.pixelSamples, p, q)) * invImageSize;

						float2 uv = rand2D();
						Ray r = scene->getCamera()->getViewRay(uv, xy2);

						// float3 sampleColor = getEnvironment(r.direction); TODO
						float3 weight(1.0f / (settings.pixelSamples * settings.pixelSamples));

						endStatTimer(stats, primaryEmit);
						StatTimer primaryPack = startStatTimer(RaytracerStatPrimaryPackCycles);

						RayStackFrame frame;
						frame.pixel = int2(x, y);
						frame.weight = weight;

						int c = (signbit(r.direction.x) << 2) | (signbit(r.direction.y) << 1) | (signbit(r.direction.z) << 0);

						radianceStack[c].frame.push_back_inbounds(frame);

						radianceStack[c].origin[0].push_back_inbounds(r.origin.x);
						radianceStack[c].origin[1].push_back_inbounds(r.origin.y);
						radianceStack[c].origin[2].push_back_inbounds(r.origin.z);

						radianceStack[c].direction[0].push_back_inbounds(r.direction.x);
						radianceStack[c].direction[1].push_back_inbounds(r.direction.y);
						radianceStack[c].direction[2].push_back_inbounds(r.direction.z);

						// Max dist is unused for primary rays
						endStatTimer(stats, primaryPack);
					}
				}
			}
		}

		// Alternate between tree traversal and shading. Shading may produce more traversal work.
		for (int generation = 0; generation < settings.maxDepth; generation++) {
			// Note: rays now have same sign bits in each direction

			for (int i = 0; i < 8; i++)
				for (int j = 0; j < (radianceStack[i].frame.size() & ~(SIMD - 1)); j += SIMD) { // TODO: handle last elements
					StatTimer pack = startStatTimer(generation == 0 ? RaytracerStatPrimaryPackCycles : RaytracerStatSecondaryPackCycles);

					PacketCollision<SIMD> result;

					const vector<float, SIMD> (&origin)[3] = {
						*(vector<float, SIMD> *)&radianceStack[i].origin[0][j],
						*(vector<float, SIMD> *)&radianceStack[i].origin[1][j],
						*(vector<float, SIMD> *)&radianceStack[i].origin[2][j]
					};

					const vector<float, SIMD> (&direction)[3] = {
						*(vector<float, SIMD> *)&radianceStack[i].direction[0][j],
						*(vector<float, SIMD> *)&radianceStack[i].direction[1][j],
						*(vector<float, SIMD> *)&radianceStack[i].direction[2][j]
					};

					// Max dist is unused for primary rays
					const vector<float, SIMD> & maxDist = vector<float, SIMD>(INFINITY); // TODO: does passing these as args work better?

					endStatTimer(stats, pack);
					StatTimer trace = startStatTimer(generation == 0 ? RaytracerStatPrimaryTraceCycles : RaytracerStatSecondaryTraceCycles);

					vector<bmask, SIMD> hit = tree.intersectPacket((KDPacketStackFrame<SIMD> *)stack, // TODO fix cast
						origin, direction, maxDist, false, result);

					endStatTimer(stats, trace);
					StatTimer shadingPack = startStatTimer(RaytracerStatShadingPackCycles);

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

							item.frame.maxDist = INFINITY;

							item.frame = radianceStack[i].frame[j + k];

							item.collision.beta = result.beta[k];
							item.collision.gamma = result.gamma[k];
							item.collision.distance = result.distance[k];
							item.collision.triangle_id = result.triangle_id[k];

							shadingBuff.push_back_inbounds(item);
						}
					}

					endStatTimer(stats, shadingPack);
				}

			for (int i = 0; i < 8; i++) {
				radianceStack[i].frame.clear();

				radianceStack[i].origin[0].clear();
				radianceStack[i].origin[1].clear();
				radianceStack[i].origin[2].clear();

				radianceStack[i].direction[0].clear();
				radianceStack[i].direction[1].clear();
				radianceStack[i].direction[2].clear();

				// Max dist is unused for primary rays
			}

			StatTimer shadingSort = startStatTimer(RaytracerStatShadingSortCycles);

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

			endStatTimer(stats, shadingSort);

			// TODO: Shorcuts for zero contribution, tiny contribution, no transparency, etc.

			for (auto & item : shadingBuff) {
				StatTimer shading = startStatTimer(RaytracerStatShadingCycles);

				const Triangle *triangle = scene->getTriangle(item.collision.triangle_id);

				// TODO: Significant cache miss here pulling vertex data in from memory. Try sorting shading work by
				// triangle?
				Vertex interp = triangle->interpolate(item.collision.beta, item.collision.gamma);
				interp.normal = normalize(interp.normal); // TODO: do we want to do this here?

				const Material *material = scene->getMaterial(triangle->material_id);

				Image<float, 4> *transparentMap = material->getTransparentTexture();
				float opacity = material->getOpacity();

				if (transparentMap) {
					Sampler sampler(Bilinear, Wrap);
					opacity = sampler.sample(transparentMap, interp.uv).w;
				}

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

				endStatTimer(stats, shading);

#if 0
				for (int l = 0; l < scene->getNumLights(); l++)
#else
				int l = (int)(rand1D() * scene->getNumLights() * 0.999f);
#endif
				{
					StatTimer shading = startStatTimer(RaytracerStatShadingCycles);

					const Light *light = scene->getLight(l);

					// TODO: Can we jitter in more than one dimensin
					// TODO: importance sampling, multiple importance sampling (need PDF probably)
					float3 lightUV = rand3D();

					float3 wi, Li;
					float r;
					light->sample(lightUV, interp.position, wi, r, Li);

					float ndotl = std::abs(dot(wi, interp.normal)); // TODO: normal mapping

					Ray shadowRay(interp.position + triangle->normal * 0.001f, wi);

					RayStackFrame shadow;
					shadow.pixel = item.frame.pixel;
					// TODO: * 20 doesn't account for forced termination?
					shadow.weight = item.frame.weight * Li * material->f(interp, wo, wi) * ndotl * scene->getNumLights() * opacity;

					endStatTimer(stats, shading);
					StatTimer shadowPack = startStatTimer(RaytracerStatShadowPackCycles);

					int c = (signbit(shadowRay.direction.x) << 2) | (signbit(shadowRay.direction.y) << 1) | (signbit(shadowRay.direction.z) << 0);

					shadowStack[c].frame.push_back_inbounds(shadow);

					shadowStack[c].origin[0].push_back_inbounds(shadowRay.origin.x);
					shadowStack[c].origin[1].push_back_inbounds(shadowRay.origin.y);
					shadowStack[c].origin[2].push_back_inbounds(shadowRay.origin.z);

					shadowStack[c].direction[0].push_back_inbounds(shadowRay.direction.x);
					shadowStack[c].direction[1].push_back_inbounds(shadowRay.direction.y);
					shadowStack[c].direction[2].push_back_inbounds(shadowRay.direction.z);

					shadowStack[c].maxDist.push_back_inbounds(r * 0.999f);

					// TODO: If light does not cast shadows, return color immediately
					endStatTimer(stats, shadowPack);
				}

				float pdf = 1.0f;
				float p_kill = 0.95f;
				bool kill = false;

				if (generation == settings.maxDepth - 1)
					kill = true;
				else if (generation >= 1 && rand1D() <= p_kill) {
					pdf *= p_kill;
					kill = true;
				}
				
				if (!kill) {
					Ray indirectRay;
					RayStackFrame indirect;

					float p_transparent = 0.0f;
					float p_indirect = 1.0f;

					if (opacity < 1.0f)
						p_transparent = 1.0f;
					// TOD: if reflection...

					float p_sum = p_transparent + p_indirect;

					if (p_sum > 0.0f) {
						StatTimer secondaryEmit = startStatTimer(RaytracerStatSecondaryEmitCycles);

						p_transparent /= p_sum;
						p_indirect /= p_sum;

						float x = rand1D();

						if (x <= p_transparent) {
							pdf *= p_transparent;

							indirectRay.origin = interp.position + item.ray.direction * 0.01f;
							indirectRay.direction = item.ray.direction;

							// Importance sampling: n dot l term cancels out
							indirect.weight = item.frame.weight * (1.0f - opacity) / pdf;
						}
						else if (x - p_transparent <= p_indirect) {
							pdf *= p_indirect;

							float2 rand = rand2D();

							indirectRay.origin = interp.position + triangle->normal * 0.001f;
							indirectRay.direction = mapCosHemisphere(1.0f, rand);
							indirectRay.direction = alignHemisphere(indirectRay.direction, triangle->normal);

							// Importance sampling: n dot l term cancels out
							indirect.weight = item.frame.weight * material->f(interp, wo, indirectRay.direction) * (float)M_PI / pdf;
						}
						else if (false) {
							indirectRay.origin = interp.position + triangle->normal * 0.001f;
							indirectRay.direction = reflect(wo, interp.normal);
							indirect.weight = item.frame.weight * material->getReflectivity(); // TODO: hack
							//float ndotl = saturate(dot(r.direction, interp.normal));
							//weight *= material->f(interp, wo, r.direction) * ndotl;
						}

						endStatTimer(stats, secondaryEmit);
						StatTimer secondaryPack = startStatTimer(RaytracerStatSecondaryPackCycles);

						indirect.pixel = item.frame.pixel;

						int c = (signbit(indirectRay.direction.x) << 2) | (signbit(indirectRay.direction.y) << 1) | (signbit(indirectRay.direction.z) << 0);

						radianceStack[c].frame.push_back_inbounds(indirect);

						radianceStack[c].origin[0].push_back_inbounds(indirectRay.origin.x);
						radianceStack[c].origin[1].push_back_inbounds(indirectRay.origin.y);
						radianceStack[c].origin[2].push_back_inbounds(indirectRay.origin.z);

						radianceStack[c].direction[0].push_back_inbounds(indirectRay.direction.x);
						radianceStack[c].direction[1].push_back_inbounds(indirectRay.direction.y);
						radianceStack[c].direction[2].push_back_inbounds(indirectRay.direction.z);

						// Max dist is unused for primary rays

						endStatTimer(stats, secondaryPack);
					}
				}
			}

			shadingBuff.clear();

			for (int p = 0; p < 3; p++) { // TODO: 3
				// TODO: Shadow rays do not generate new rays, so we only need to iterate once
				for (int i = 0; i < 8; i++)
					for (int j = 0; j < (shadowStack[i].frame.size() & ~(SIMD - 1)); j += SIMD) { // TODO: handle last elements
						PacketCollision<SIMD> result;

						// TODO: Avoid this copy by passing registers directly
						const vector<float, SIMD> (&origin)[3] = {
							*(vector<float, SIMD> *)&shadowStack[i].origin[0][j],
							*(vector<float, SIMD> *)&shadowStack[i].origin[1][j],
							*(vector<float, SIMD> *)&shadowStack[i].origin[2][j]
						};

						const vector<float, SIMD> (&direction)[3] = {
							*(vector<float, SIMD> *)&shadowStack[i].direction[0][j],
							*(vector<float, SIMD> *)&shadowStack[i].direction[1][j],
							*(vector<float, SIMD> *)&shadowStack[i].direction[2][j]
						};

						const vector<float, SIMD> & maxDist = *(vector<float, SIMD> *)&shadowStack[i].maxDist[j];

						StatTimer shadowTrace = startStatTimer(RaytracerStatShadowTraceCycles);

						vector<bmask, SIMD> hit = tree.intersectPacket((KDPacketStackFrame<SIMD> *)stack, // TODO fix cast
							origin, direction, maxDist, true, result);

						endStatTimer(stats, shadowTrace);
						StatTimer updateFramebuffer = startStatTimer(RaytracerStatUpdateFramebufferCycles);

						// TODO: Might be useful to pass in active mask

						// TODO: Do in SIMD
						for (int k = 0; k < SIMD; k++) {
							if (!hit[k]) {
								const RayStackFrame & frame = shadowStack[i].frame[j + k];

								float3 color = output->getPixel(frame.pixel.x, frame.pixel.y).xyz();
								color = color + frame.weight; // TODO
								output->setPixel(frame.pixel.x, frame.pixel.y, float4(color, 1.0f));
							}
							else {
								// TODO: Terminate eventually

								ShadingWorkItem item;

								item.ray.origin[0] = shadowStack[i].origin[0][j + k];
								item.ray.origin[1] = shadowStack[i].origin[1][j + k];
								item.ray.origin[2] = shadowStack[i].origin[2][j + k];

								item.ray.direction[0] = shadowStack[i].direction[0][j + k];
								item.ray.direction[1] = shadowStack[i].direction[1][j + k];
								item.ray.direction[2] = shadowStack[i].direction[2][j + k];

								item.frame.maxDist = shadowStack[i].maxDist[j + k];

								item.frame = shadowStack[i].frame[j + k];

								item.collision.beta = result.beta[k];
								item.collision.gamma = result.gamma[k];
								item.collision.distance = result.distance[k];
								item.collision.triangle_id = result.triangle_id[k];

								shadowBuff.push_back_inbounds(item);
							}
						}

						endStatTimer(stats, updateFramebuffer);
					}

				for (int i = 0; i < 8; i++) {
					shadowStack[i].frame.clear();

					shadowStack[i].origin[0].clear();
					shadowStack[i].origin[1].clear();
					shadowStack[i].origin[2].clear();

					shadowStack[i].direction[0].clear();
					shadowStack[i].direction[1].clear();
					shadowStack[i].direction[2].clear();

					shadowStack[i].maxDist.clear();
				}

				for (auto & item : shadowBuff) {
					// TODO: Only do this for transparent objects/triangles
					const Triangle *triangle = scene->getTriangle(item.collision.triangle_id);

					// TODO: Significant cache miss here pulling vertex data in from memory. Try sorting shading work by
					// triangle?
					// TODO: We only need the UV. Also, in general, position can be created from O + tD, which is probably
					// cheaper than interpolating.

					float alpha = 1.0f - item.collision.beta - item.collision.gamma;
					float3 position = item.ray.origin + item.collision.distance * item.ray.direction;
					float2 uv = triangle->v[0].uv * alpha + triangle->v[1].uv * item.collision.beta + triangle->v[2].uv * item.collision.gamma;

					//Vertex interp = triangle->interpolate(item.collision.beta, item.collision.gamma);
					//interp.normal = normalize(interp.normal); // TODO: do we want to do this here?

					const Material *material = scene->getMaterial(triangle->material_id);

					Image<float, 4> *transparentMap = material->getTransparentTexture();
					float opacity = material->getOpacity();

					if (transparentMap) {
						Sampler sampler(Bilinear, Wrap);
						opacity = sampler.sample(transparentMap, uv).w;
					}

					if (opacity < 1.0f) {
						Ray shadowRay;
						RayStackFrame shadow;

						shadowRay.origin = position + item.ray.direction * 0.01f;
						shadowRay.direction = item.ray.direction;

						// Importance sampling: n dot l term cancels out
						shadow.weight = item.frame.weight * (1.0f - opacity);

						shadow.pixel = item.frame.pixel;

						int c = (signbit(shadowRay.direction.x) << 2) | (signbit(shadowRay.direction.y) << 1) | (signbit(shadowRay.direction.z) << 0);

						shadowStack[c].frame.push_back_inbounds(shadow);

						shadowStack[c].origin[0].push_back_inbounds(shadowRay.origin.x);
						shadowStack[c].origin[1].push_back_inbounds(shadowRay.origin.y);
						shadowStack[c].origin[2].push_back_inbounds(shadowRay.origin.z);

						shadowStack[c].direction[0].push_back_inbounds(shadowRay.direction.x);
						shadowStack[c].direction[1].push_back_inbounds(shadowRay.direction.y);
						shadowStack[c].direction[2].push_back_inbounds(shadowRay.direction.z);

						shadowStack[c].maxDist.push_back_inbounds(item.frame.maxDist - item.collision.distance);
					}
				}

				shadowBuff.clear();
			}
		}

        // TODO: Flushing one tile at a time keeps the tile in the cache probably, but might
        // not get the most coherence. Adjusting the tile size would affect this probably.

		endStatTimer(stats, totalCycles);
    }

    //std::cout << "Ray buffer size: " << rayBuff.capacity() << " (" << (rayBuff.capacity() * sizeof(Ray) + 1024 - 1) / 1024 << "kb)" << std::endl;

    numThreadsAlive--;
}

bool Raytracer::intersect(float2 uv, Collision & result) {
	KDStackFrame stack[64];

	Ray r = scene->getCamera()->getViewRay(float2(0, 0), uv);

	return tree.intersect(stack, r, INFINITY, result);
}
