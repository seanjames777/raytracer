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
    RaytracerSettings settings;

    Camera *camera = new Camera(Vec3(-25, 25, -25), Vec3(0, 2.0f, 0), (float)settings.width / (float)settings.height,
        M_PI / 3.4f, 19.25f, 0.0f);

    Bitmap *output = new Bitmap(settings.width, settings.height, 3);

    Bitmap *environment = Bitmap::load("content/textures/cubemap.bmp");

    Scene *scene = new Scene(camera, output, environment);

    Material *mat = new Material(Vec3(0.00f, 0.00f, 0.00f), Vec3(1.0f, 1.0f, 1.0f),
        Vec3(1.0f, 1.0f, 1.0f) * 0, 8.0f, 0.0f, 0.0f, 10.0f);

    std::vector<Polygon> polys;
    FbxLoader::load("content/models/dragon.fbx", polys);
    for (int i = 0; i < polys.size(); i++)
        scene->addPoly(&polys[i], mat);

    Light *light1 = new PointLight(Vec3(-10, 30, 0), Vec3(1.0f, 1.0f, 1.0f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light1);

    //Light *light2 = new PointLight(Vec3(4, 9, -4), Vec3(0.2f, 0.2f, 0.2f), 0.0f, 15.0f, 0.5f, false);
    //scene->addLight(light2);

    GLImageDisplay *disp = new GLImageDisplay(1024, 1024, output);

    Raytracer *rt = new Raytracer(settings, scene);
    rt->startThreads();

    int nFrames = 1;
    for (int i = 0; i < nFrames; i++) {
        float theta = ((float)i / (float)nFrames + .25f) * 2.0f * M_PI;

        std::stringstream ss;
        ss << "output/frame" << i << ".bmp";

        //camera->setPosition(Vec3(cosf(theta) * 15.0f, 10.0f, sinf(theta) * 15.0f));
        rt->render(disp);
        output->save(ss.str());
    }

    rt->stopThreads();

    return 0;
}
