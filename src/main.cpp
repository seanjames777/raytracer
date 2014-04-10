/**
 * @file main.cpp
 *
 * @brief Entrypoint
 *
 * @author Sean James
 */

#include <defs.h>
#include <scene.h>
#include <sphere.h>
#include <plane.h>
#include <pointlight.h>
#include <raytracer.h>
#include <glimagedisplay.h>
#include <fbxloader.h>

int main(int argc, char *argv[]) {
    int width = 1024;
    int height = 1024;

    Camera *camera = new Camera(Vec3(-0.01f, 5.01f, -15), Vec3(0, 5.0f, 0), (float)width / (float)height,
        M_PI / 3.4f, 19.25f, 0.0f);

    Bitmap *output = new Bitmap(width, height, 3);

    Bitmap *environment = Bitmap::load("content/textures/cubemap.bmp");
    environment = NULL;

    Scene *scene = new Scene(camera, output, environment);

    Material *mat = new Material(Vec3(0.2f, 0.2f, 0.2f), Vec3(0.8f, 0.8f, 0.8f),
        Vec3(1.0f, 1.0f, 1.0f) * 0, 8.0f, 0.0f, 0.0f, 10.0f);

    Sphere *sphere1 = new Sphere(Vec3(2, 2, 2), 2.0f);
    scene->addShape(sphere1, mat);

    Sphere *sphere2 = new Sphere(Vec3(-2, 2, -2), 2.0f);
    scene->addShape(sphere2, mat);

    //Plane *plane = new Plane(Vec3(0, 1, 0), 0.0f, Vec3(0, 0, 1), 100.0f, 100.0f);
    //scene->addShape(plane, mat);

    std::vector<Poly> polys;
    FbxLoader::load("content/models/box.fbx", polys);
    for (int i = 0; i < polys.size(); i++)
        scene->addShape(&polys[i], mat);

    Light *light1 = new PointLight(Vec3(0, 9, 0), Vec3(1.0f, 1.0f, 1.0f), 0.1f, 15.0f, 0.5f, true);
    scene->addLight(light1);

    //Light *light2 = new PointLight(Vec3(4, 9, -4), Vec3(0.2f, 0.2f, 0.2f), 0.0f, 15.0f, 0.5f, false);
    //scene->addLight(light2);

    GLImageDisplay *disp = new GLImageDisplay(1024, 1024, output);

    Raytracer *rt = new Raytracer(scene);
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

    getchar();
    return 0;
}
