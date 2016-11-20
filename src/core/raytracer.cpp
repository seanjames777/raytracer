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
		void *nodeMem = aligned_alloc(1024 * 1024 * 100, alignof(KDNode));
		void *triangleMem = aligned_alloc(1024 * 1024 * 100, alignof(SetupTriangle));

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
				float3 weight(1.0f / (settings.pixelSamples * settings.pixelSamples));

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
						float3 sampleColor;

						Collision result;

						if (tree->intersect(stack, r, false, result)) {
							Shader *shader = scene->getShader(result.triangle_id);
							sampleColor = shader->shade(stack, tree, 0, r, result, scene, this);
						}

						color += sampleColor * weight;
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
