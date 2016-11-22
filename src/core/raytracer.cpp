/**
 * @file core/raytracer.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/raytracer.h>

#include <iostream>

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

    nBlocksW = (scene->getOutput()->getWidth() + settings.blockSize - 1) / settings.blockSize;
    nBlocksH = (scene->getOutput()->getHeight() + settings.blockSize - 1) / settings.blockSize;
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

    while(!shouldShutdown) {
        blockID = currBlockID++;

        if (blockID >= nBlocks)
            break;

        int y = blockID / nBlocksW;
        int x = blockID % nBlocksW;

        int x0 = settings.blockSize * x;
        int y0 = settings.blockSize * y;

        for (int y = y0; y < y0 + settings.blockSize; y++) {
            for (int x = x0; x < x0 + settings.blockSize; x++) {
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

						Collision result;

						// float3 sampleColor = getEnvironment(r.direction); TODO
						float3 sampleColor;
						float3 weight(1.0f / (settings.pixelSamples * settings.pixelSamples));

						for (int depth = 0; depth < settings.maxDepth; depth++) {
							if (tree->intersect(stack, r, false, INFINITY, result)) {
								const Triangle *triangle = scene->getTriangle(result.triangle_id);

								// TODO: Significant cache miss here pulling vertex data in from memory. Try sorting shading work by
								// triangle?
								Vertex interp = triangle->interpolate(result.beta, result.gamma);

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

								float3 wo = -r.direction;

								for (int l = 0; l < scene->getNumLights(); l++) {
									const Light *light = scene->getLight(l);

									float3 wi, Li;
									float r;
									light->sample(interp.position, wi, r, Li);

									Ray shadow(interp.position + triangle->normal * 0.001f, wi);

									if (!light->castsShadows() || !tree->intersect(stack, shadow, true, r, result)) {
										float ndotl = saturate(dot(wi, interp.normal)); // TODO: normal mapping

										sampleColor += weight * Li * material->f(interp, wo, wi) * ndotl;
										//sampleColor += weight * material->f(interp, wo, wi);
									}
								}

								r.origin = interp.position + triangle->normal * 0.001f;

								float secondary;
								rand1D(1, &secondary);

								if (secondary < 0.5f || material->getReflectivity() == 0.0f) {
									float2 rand;
									rand2D(1, &rand);

									mapSamplesCosHemisphere(1, 1.0f, &rand, &r.direction);
									alignHemisphereNormal(1, &r.direction, triangle->normal);

									// Importance sampling: n dot l term cancels out
									weight *= material->f(interp, wo, r.direction) * (float)M_PI;
								}
								else {
									r.direction = reflect(wo, interp.normal);
									weight *= material->getReflectivity(); // TODO: hack
									//float ndotl = saturate(dot(r.direction, interp.normal));
									//weight *= material->f(interp, wo, r.direction) * ndotl;
								}
							}
							else
								break;
						}

						color += sampleColor;
                    }
                }

				scene->getOutput()->setPixel(x, y, color);
            }
        }

        // TODO: Flushing one tile at a time keeps the tile in the cache probably, but might
        // not get the most coherence. Adjusting the tile size would affect this probably.
    }

    //std::cout << "Ray buffer size: " << rayBuff.capacity() << " (" << (rayBuff.capacity() * sizeof(Ray) + 1024 - 1) / 1024 << "kb)" << std::endl;

    numThreadsAlive--;
}
