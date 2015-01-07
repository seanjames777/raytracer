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

int main(int argc, char *argv[]) {
    std::cout << "PID: " << getpid() << std::endl;

    RaytracerSettings settings;
    settings.width = 2048;
    settings.height = 1080;
    settings.pixelSamples = 4;
    settings.occlusionSamples = 0;
    settings.occlusionDistance = 10.0f;
    settings.shadowSamples = 0;

    float aspect = (float)settings.width / (float)settings.height;
    Camera *camera = new Camera(vec3(-19, 10, -20), vec3(0, 5.0f, 0), aspect,
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
    FbxLoader::load(
        PathUtil::prependExecutableDirectory("content/models/sphere.fbx"),
        polys, vec3(0, 0, 0), vec3(-M_PI / 2.0f, 0, 0), vec3(1.2f, 1.2f, 1.2f));
    for (int i = 0; i < polys.size(); i++)
        scene->addPoly(polys[i], diffuse);

    polys.clear();
    FbxLoader::load(
        PathUtil::prependExecutableDirectory("content/models/plane.fbx"),
        polys, vec3(0, 0, 0), vec3(0, 0, 0), vec3(3, 1, 3));
    for (int i = 0; i < polys.size(); i++)
        scene->addPoly(polys[i], diffuse);

    Light *light1 = new PointLight(vec3(-20, 20, -20), vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light1);

    Light *light2 = new PointLight(vec3(20, 20, 20), vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light2);

    GLImageDisplay *disp = new GLImageDisplay(1024 * aspect, 1024, output);

    Raytracer *rt = new Raytracer(settings, scene);
    // TODO: thread pool

    Timer timer;

    int nFrames = 1;
    for (int i = 0; i < nFrames; i++) {
        float theta = ((float)i / (float)nFrames + .25f) * 2.0f * M_PI;
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

    return 0;
}
