/**
 * @file main.cpp
 *
 * @brief Entrypoint
 *
 * @author Sean James
 */

#include <scene.h>
#include <pointlight.h>
#include <raytracer.h>
#include <glimagedisplay.h>
#include <fbxloader.h>
#include <path.h>
#include <sstream>
#include <bmpimage.h>

RaytracerSettings settings;
bool wdisplay = true;

void parseArgs(int argc, char *argv[]) {
    settings.width = 2048;
    settings.height = 1080;
    settings.pixelSamples = 4;
    settings.occlusionSamples = 4;
    settings.occlusionDistance = 5.0f;
    settings.shadowSamples = 4;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-nodisplay") == 0)
            wdisplay = false;
        else if (strcmp(argv[i], "-w") == 0)
            settings.width = atoi(argv[++i]);
        else if (strcmp(argv[i], "-h") == 0)
            settings.height = atoi(argv[++i]);
        else if (strcmp(argv[i], "-samples") == 0)
            settings.pixelSamples = atoi(argv[++i]);
        // TODO
    }
}

int main(int argc, char *argv[]) {
    parseArgs(argc, argv);

    float aspect = (float)settings.width / (float)settings.height;

    Camera *camera = new Camera(vec3(-80, 25.0f, -80), vec3(0, 5.0f, 0), aspect,
        M_PI / 3.4f, 19.25f, 0.0f);

    std::shared_ptr<Image> output = std::make_shared<Image>(settings.width, settings.height);

    std::shared_ptr<Image> environment = BMPImage::loadBMP(
        PathUtil::prependExecutableDirectory("content/textures/cubemap.bmp"));

    std::shared_ptr<Image> checker = BMPImage::loadBMP(
        PathUtil::prependExecutableDirectory("content/textures/checker.bmp"));

    //environment->applyGamma(2.2f);
    //checker->applyGamma(2.2f);

    std::shared_ptr<Sampler> check_sampler = std::make_shared<Sampler>(
        Linear, Linear, Wrap);

    std::shared_ptr<Sampler> env_sampler = std::make_shared<Sampler>(
        Nearest, Nearest, Wrap);

    Scene *scene = new Scene(camera, output, env_sampler, environment);

    Material *diffuse = new Material(vec3(0.1f, 0.1f, 0.1f), vec3(0.9f, 0.9f, 0.9f),
        vec3(0.3f, 0.3f, 0.3f), 8.0f, 0.0f, 0.0f, 10.0f, check_sampler, checker);

    Material *chrome = new Material(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),
        vec3(1.0f, 1.0f, 1.0f), 16.0f, 0.0f, 0.0f, 10.0f, check_sampler, nullptr);

    std::vector<Triangle> polys;

    for (int z = -2; z <= 2; z++) {
        for (int x = -2; x <= 2; x++) {
            for (int y = -2; y <= 2; y++) {
                polys.clear();

                /*FbxLoader::load(
                    PathUtil::prependExecutableDirectory("content/models/box.fbx"),
                    polys, vec3(0.0f, 2.5f, 0.0f), vec3(0.0f), vec3(5.0f));*/
                FbxLoader::load(
                    PathUtil::prependExecutableDirectory("content/models/dragon.fbx"),
                    polys, vec3(x * 20.0f, y * 20.0f, z * 20.0f), vec3(0.0f), vec3(1.0f));

                for (int i = 0; i < polys.size(); i++)
                    scene->addPoly(polys[i], diffuse);
            }
        }
    }

    polys.clear();
    FbxLoader::load(
        PathUtil::prependExecutableDirectory("content/models/plane.fbx"),
        polys, vec3(0, 0, 0), vec3(0, 0, 0), vec3(3, 1, 3));
    for (size_t i = 0; i < polys.size(); i++)
        scene->addPoly(polys[i], diffuse);

    Light *light1 = new PointLight(vec3(-20, 20, -20), vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light1);

    Light *light2 = new PointLight(vec3(20, 20, 20), vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light2);

    GLImageDisplay *disp = nullptr;

    if (wdisplay)
        disp = new GLImageDisplay((int)(1024 * aspect), 1024, output);

    Raytracer *rt = new Raytracer(settings, scene);
    // TODO: thread pool

    Timer timer;

    int nFrames = 1;
    for (int i = 0; i < nFrames; i++) {
        float theta = ((float)i / (float)nFrames + .25f) * 2.0f * (float)M_PI;
        //camera->setPosition(vec3(cosf(theta) * 15.0f, 10.0f, sinf(theta) * 15.0f));
        rt->render(disp);

        std::stringstream ss;

        /*ss << "output/frame" << i << ".exr";
        output->saveEXR(ss.str());*/

        //ss.str("");
        //ss << "output/frame" << i << ".bmp";
        //output->applyTonemapping(2.0f);
        //output->applyGamma(1.0f / 2.2f);
        //output->saveBMP(ss.str());
    }

    printf("Done: %f seconds (total), %f seconds (CPU)\n",
        timer.getElapsedMilliseconds() / 1000.0,
        timer.getCPUTime() / 1000.0);

    getchar();

    return 0;
}
