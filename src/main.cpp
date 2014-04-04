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
#include <pointlight.h>
#include <raytracer.h>
#include <glimagedisplay.h>
#include <fbxloader.h>

int main(int argc, char *argv[]) {
    int width = 1024;
    int height = 1024;

    Camera *camera = new Camera(Vec3(-15, 15, -15), Vec3(0, 3, 0), (float)width / (float)height,
        M_PI / 3.4f, 1.0f, 0.0f);
    Bitmap *output = new Bitmap(width, height, 3);
    Scene *scene = new Scene(camera, output);

    Material *mat = new Material(Vec3(0.1f, 0.1f, 0.1f), Vec3(0.9f, 0.9f, 0.9f),
        Vec3(1.0f, 1.0f, 1.0f) * .15f, 8.0f, 0.0f, 0.0f, 1.0f);

    /*for (int z = -1; z <= 1; z++)
        for (int x = -1; x <= 1; x++)
            for (int y = 0; y <= 0; y++) {
                Sphere *sphere = new Sphere(Vec3(x * 4, y * 4, z * 4), 1.2f);
                scene->addShape(sphere, mat);
            }*/

    std::vector<Poly> polys;
    FbxLoader::load("content/models/dragon.fbx", polys);
    for (int i = 0; i < polys.size(); i++)
        scene->addShape(&polys[i], mat);

    Light *light1 = new PointLight(Vec3(100, 100, 0), Vec3(0.94f, 0.81f, 0.7f), 0.0f, 150.0f, 0.0f, false);
    scene->addLight(light1);

    Light *light2 = new PointLight(Vec3(-100, 2, 0), Vec3(0.5f, 0.55f, 0.58f), 0.0f, 150.0f, 0.0f, false);
    scene->addLight(light2);

    GLImageDisplay *disp = new GLImageDisplay(1024, 1024, output);

    Raytracer *rt = new Raytracer(scene);
    rt->render(disp);

    output->save("output.bmp");

    return 0;
}
