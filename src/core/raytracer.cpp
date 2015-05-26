/**
 * @file core/raytracer.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/raytracer.h>

Raytracer::Raytracer(RaytracerSettings settings, Scene *scene)
    : settings(settings),
      scene(scene)
{
    srand((unsigned)time(0));
}

Raytracer::~Raytracer() {
}

void Raytracer::render() {
    should_shutdown = false;

    KDSAHBuilder builder;
    tree = builder.build(scene->getTriangles(), &stats);

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
        should_shutdown = true;

    for (auto& worker : workers)
        worker->join();

    workers.clear();
}

void Raytracer::worker_thread(int idx, int numThreads) {
    // Allocate a reusable KD traversal stack for each thread. Uses statistics computed
    // during tree construction to preallocate a stack with the worst case depth/size to avoid
    // bounds checks/dynamic resizing during rendering.
    util::stack<KDStackFrame> kdStack(stats.max_depth);
    RayBuffer rayBuff;

    int width = scene->getOutput()->getWidth();
    int height = scene->getOutput()->getHeight();

    vec2 invImageSize(1.0f / (float)width, 1.0f / (float)height);
    float sampleContrib = 1.0f / (float)(settings.pixelSamples * settings.pixelSamples);

    int blockID = idx;

    while(!should_shutdown) {
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

                // Take jittered sampled to reduce variance and move from stairstepping
                // artifacts to noise
                #define MAX_PIXEL_SAMPLES 16
                vec2 samples[MAX_PIXEL_SAMPLES * MAX_PIXEL_SAMPLES];
                randJittered2D(settings.pixelSamples, samples);

                // tODO: Is the pointer chasing through scene bad?
                scene->getOutput()->setPixel(x, y, vec4(0.0f, 0.0f, 0.0f, 1.0f));

                // TODO: It's possible to do better sampling

                vec2 xy = vec2(x, y);

                for (int p = 0; p < settings.pixelSamples; p++) {
                    for (int q = 0; q < settings.pixelSamples; q++) {
                        vec2 uv = (xy + samples[p * settings.pixelSamples + q]) * invImageSize;

                        Ray r = scene->getCamera()->getViewRay(uv, vec3(sampleContrib), (short)x, (short)y, 1);

                        // TODO: Super slow possibly. Fixed size queue? Could preallocate enough
                        // rays for samples? Or, could alternate between filling and draining
                        // when queue is full?
                        // TODO: Makes a copy
                        rayBuff.enqueue(r);
                    }
                }
            }
        }

        // TODO: Flushing one tile at a time keeps the tile in the cache probably, but might
        // not get the most coherence. Adjusting the tile size would affect this probably.

        while (!rayBuff.empty()) {
            Ray r = rayBuff.dequeue(); // TODO: Makes a copy

            // TODO: Might be worth skipping rays with really tiny contributions

            if (r.mode == Shade) {
                // vec3 sampleColor = getEnvironment(r.direction); TODO
                vec3 sampleColor;

                Collision result;

                if (tree->intersect(kdStack, r, result)) {
                    Shader *shader = scene->getShader(result.triangle_id);
                    sampleColor = shader->shade(rayBuff, r, result, scene, this);
                }

                vec4 color = scene->getOutput()->getPixel(r.px, r.py);
                color += vec4(sampleColor * r.weight, 1.0f); // TODO: Bogus alpha
                scene->getOutput()->setPixel(r.px, r.py, color);
            }
            else {
                Collision result;

                if (!tree->intersect(kdStack, r, result)) {
                    vec4 color = scene->getOutput()->getPixel(r.px, r.py);
                    color += vec4(r.weight, 1.0f); // TODO: Bogus alpha
                    scene->getOutput()->setPixel(r.px, r.py, color);
                }
            }

            // TODO: Under this decomposition, many rays might have zero
            // contribution because they defer work to secondary rays
        }

        // blockID += numThreads;
    }

    std::cout << "Ray buffer size: " << rayBuff.capacity() << " (" << (rayBuff.capacity() * sizeof(Ray) + 1024 - 1) / 1024 << "kb)" << std::endl;

    numThreadsAlive--;
}
