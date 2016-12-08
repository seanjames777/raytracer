/**
 * @file main.cpp
 *
 * @brief Raytracer demo application entry point
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/raytracer.h>
#include <util/imagedisplay.h>
#include <scenes/sponzascene.h>
#include <scenes/simplescene.h>

extern void testVectors();

int main(int argc, char *argv[]) {
#ifndef NDEBUG
    printf("DEBUG build\n");
#else
    printf("RELEASE build\n");
#endif

	testVectors();

    printf("Loading scene...\n");

    RaytracerSettings settings;
    settings.width = 1920;
    settings.height = 1080;
    settings.pixelSamples = 16;
    settings.numThreads = std::thread::hardware_concurrency() - 1; // TODO
	settings.maxDepth = 20;

    auto output = new Image<float, 4>(settings.width, settings.height);

    Scene *scene = nullptr;

    switch (0) {
    case 0:
    default:
        scene = new SponzaScene();
        break;
    case 1:
        scene = new SimpleScene();
        break;
    }

    printf("%lu polygons, %lu lights\n", scene->getTriangles().size(), scene->getNumLights());

    auto rt = new Raytracer(settings, scene, output);
	auto disp = new ImageDisplay(1920, 1080, output);

    printf("Rendering\n");

    Timer timer;
    timer.reset();

    rt->render();

    bool finished = false;

    while (!disp->shouldClose()) {
        disp->refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));

#if 0
        Collision result;
        float2 cursor = disp->getCursorPos() / float2(settings.width, settings.height);

        if (rt->intersect(cursor, result))
            std::cout << result.triangle_id << " " << scene->getTriangle(result.triangle_id)->material_id << " " << result.distance << std::endl;
        else
            std::cout << "miss" << std::endl;
#endif

        if (!finished && rt->finished()) {
            finished = true;
            
            // TODO: Move into raytracer itself
            float elapsed = (float)timer.getElapsedMilliseconds() / 1000.0f;
            float cpu     = (float)timer.getCPUTime() / 1000.0f;

            printf("Done: %f seconds (total), %f seconds (CPU), speedup: %.02f\n",
                elapsed, cpu, cpu / elapsed);

			RaytracerStats stats;
			rt->shutdown(true, &stats);

			unsigned long longestName = 0;

			for (int i = 0; i < RaytracerStatCount; i++)
				longestName = max(strlen(RaytracerStatNames[i]), longestName);

			for (int i = 0; i < RaytracerStatCount; i++) {
				printf("%s:", RaytracerStatNames[i]);

				int len = strlen(RaytracerStatNames[i]);

				for (int j = 0; j < longestName - len; j++)
					printf(" ");

				printf("%16llu (%6.02f %%)\n", stats.stat[i], (float)stats.stat[i] / (float)stats.stat[0] * 100);
			}
        }
    }

	if (!finished)
		rt->shutdown(false);

    return 0;
}
