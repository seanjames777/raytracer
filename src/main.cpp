/**
 * @file main.cpp
 *
 * @brief Entrypoint
 *
 * @author Sean James
 */

#include <defs.h>
#include <scene.h>
#include <pointlight.h>
#include <raytracer.h>
#include <glimagedisplay.h>
#include <fbxloader.h>

int main(int argc, char *argv[]) {
    std::cout << "PID: " << getpid() << std::endl;

    Timer timer;

    RaytracerSettings settings;
    settings.width = 1920;
    settings.height = 1080;
    settings.pixelSamples = 4;
    settings.occlusionSamples = 4;
    settings.occlusionDistance = 15.0f;
    settings.shadowSamples = 4;

    float aspect = (float)settings.width / (float)settings.height;
    Camera *camera = new Camera(Vec3(-14, 15, -16), Vec3(0, 4.0f, 0), aspect,
        M_PI / 3.4f, 19.25f, 0.0f);

    Image *output = new Image(settings.width, settings.height);

    Image *environment = Image::loadBMP("content/textures/cubemap.bmp");
    environment->applyGamma(2.2f);

    Scene *scene = new Scene(camera, output, environment);

    Material *mat = new Material(Vec3(0.1f, 0.1f, 0.1f), Vec3(0.9f, 0.9f, 0.9f),
        Vec3(1.0f, 1.0f, 1.0f), 8.0f, 0.0f, 0.0f, 10.0f);

    std::vector<Polygon> polys;
    FbxLoader::load("content/models/bunny_high.fbx", polys);
    FbxLoader::load("content/models/plane.fbx", polys);

    for (int i = 0; i < polys.size(); i++)
        scene->addPoly(polys[i], mat);

    Light *light1 = new PointLight(Vec3(-15, 15, 15), Vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light1);

    Light *light2 = new PointLight(Vec3(15, 15, 15), Vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light2);

    GLImageDisplay *disp = new GLImageDisplay(1024 * aspect, 1024, output);

    Raytracer *rt = new Raytracer(settings, scene);
    rt->startThreads();

    int nFrames = 1;
    for (int i = 0; i < nFrames; i++) {
        float theta = ((float)i / (float)nFrames + .25f) * 2.0f * M_PI;
        //camera->setPosition(Vec3(cosf(theta) * 15.0f, 10.0f, sinf(theta) * 15.0f));
        rt->render(disp);

        std::stringstream ss;

        ss << "output/frame" << i << ".exr";
        output->saveEXR(ss.str());

        ss.str("");
        ss << "output/frame" << i << ".bmp";
        output->applyTonemapping(2.0f);
        output->applyGamma(1.0f / 2.2f);
        output->saveBMP(ss.str());
    }

    rt->stopThreads();

    printf("Done: %f seconds (total), %f seconds (CPU)\n",
        timer.getElapsedMilliseconds() / 1000.0,
        timer.getCPUTime() / 1000.0);

    return 0;
}
