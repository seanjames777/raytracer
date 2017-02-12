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
#include <scenes/cornellscene.h>

extern void testVectors();

int main(int argc, char *argv[]) {
    RaytracerSettings settings;
    settings.width = 1920;
    settings.height = 1080;
    settings.pixelSamples = 4;
    settings.numThreads = std::thread::hardware_concurrency() - 1; // TODO
	settings.maxDepth = 20;

    int sceneIndex = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [--width <width>] [--height <height>] [--samples <samples>] [--scene <scene>]\n", argv[0]);
            printf("\n");
            printf("Scenes:\n");
            printf("    0: Sponza\n");
            printf("    1: Simple\n");
            printf("    2: Cornell\n");
            return 0;
        }
        else if (strcmp(argv[i], "--width") == 0)
            settings.width = atoi(argv[++i]);
        else if (strcmp(argv[i], "--height") == 0)
            settings.height = atoi(argv[++i]);
        else if (strcmp(argv[i], "--samples") == 0)
            settings.pixelSamples = atoi(argv[++i]);
        else if (strcmp(argv[i], "--scene") == 0)
            sceneIndex = atoi(argv[++i]);
        else {
            printf("Unknown argument '%s'\n", argv[i]);
            return 1;
        }
    }

    #ifndef NDEBUG
    printf("DEBUG build\n");
#else
    printf("RELEASE build\n");
#endif

    //testVectors();

    printf("Loading scene...\n");

    auto output = new Image<float, 4>(settings.width, settings.height);

    Scene *scene = nullptr;

    switch (sceneIndex) {
    case 0:
    default:
        scene = new SponzaScene();
        break;
    case 1:
        scene = new SimpleScene();
        break;
    case 2:
        scene = new CornellScene();
        break;
    }

    //printf("%lu polygons, %lu lights\n", scene->getTriangles().size(), scene->getNumLights());

    auto rt = new Raytracer(settings, scene, output);
	auto disp = new ImageDisplay(settings.width, settings.height, output);

    printf("Rendering\n");

    Timer timer;
    timer.reset();

    PVScene pvscene(scene);

    while (!disp->shouldClose()) {
        disp->drawPreviewScene(&pvscene);
        disp->swap();
    }

    getchar();

    rt->render();

    bool finished = false;

    while (!disp->shouldClose()) {
        disp->refresh();
        disp->swap();

#if 0
        Collision result;
        float2 cursor = disp->getCursorPos() / float2(settings.width, settings.height);
        std::cout << disp->getCursorPos() << std::endl;

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
