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
#include <fbxloader.h>
#include <util/path.h>
#include <sstream>
#include <bmpimage.h>
#include <string.h>
#include <net/protocol.h>
#include <math/matrix.h>
#include <util/glimagedisplay.h>

std::shared_ptr<GLImageDisplay> disp;
std::shared_ptr<Image> output;
Raytracer *rt;
bool started = false;
Timer timer;

Vertex transform_vertex(const Vertex & vertex, const mat4x4 & transform,
    const mat4x4 & transformInverseTranspose)
{
    vec4 position = transform * vec4(vertex.position, 1.0f);
    vec4 normal = transformInverseTranspose * vec4(vertex.normal, 0.0f);

    return Vertex(position.xyz(), normalize(normal.xyz()), vertex.uv, vertex.color);
}

void transform_mesh(const std::vector<Triangle> & src, std::vector<Triangle> & dst,
    vec3 translation_v, vec3 rotation_v, vec3 scale_v)
{
    mat4x4 transform =
        translation(translation_v.x, translation_v.y, translation_v.z) *
        yawPitchRoll(rotation_v.y, rotation_v.x, rotation_v.z) *
        scale(scale_v.x, scale_v.y, scale_v.z);

    mat4x4 transformInverseTranspose = transpose(inverse(transform));

    for (auto & tri : src) {
        dst.push_back(Triangle(
            transform_vertex(tri.v1, transform, transformInverseTranspose),
            transform_vertex(tri.v2, transform, transformInverseTranspose),
            transform_vertex(tri.v3, transform, transformInverseTranspose)
        ));
    }
}

int main(int argc, char *argv[]) {
    RaytracerSettings settings;
    settings.width = 1920;
    settings.height = 1080;
    settings.pixelSamples = 4;
    settings.occlusionSamples = 5;
    settings.occlusionDistance = 4.0f;
    settings.shadowSamples = 4;

    float aspect = (float)settings.width / (float)settings.height;

    //Camera *camera = new Camera(vec3(-80, 25.0f, -80), vec3(0, 5.0f, 0), aspect,
    //    (float)M_PI / 3.4f, 19.25f, 0.0f);
    Camera *camera = new Camera(vec3(-15.0f, 10.0f, -15.0f), vec3(0.0f, 3.0f, 0.0f), aspect,
        (float)M_PI / 3.4f, 19.25f, 0.0f);

    output = std::make_shared<Image>(settings.width, settings.height);

    std::shared_ptr<Image> environment = BMPImage::loadBMP(
        util::prependExecutableDirectory("content/textures/cubemap.bmp"));

    std::shared_ptr<Image> checker = BMPImage::loadBMP(
        util::prependExecutableDirectory("content/textures/checker.bmp"));

    //environment->applyGamma(2.2f);
    //checker->applyGamma(2.2f);

    std::shared_ptr<Sampler> check_sampler = std::make_shared<Sampler>(
        Linear, Linear, Wrap);

    std::shared_ptr<Sampler> env_sampler = std::make_shared<Sampler>(
        Nearest, Nearest, Wrap);

    Scene *scene = new Scene(camera, output, env_sampler, environment);

    Material *ground = new PBRMaterial();

    Material *bunny = new PBRMaterial();

    std::vector<Triangle> polys, transformed;

    /*FbxLoader::load(
    util::prependExecutableDirectory("content/models/box.fbx"),
    polys, vec3(0.0f, 2.5f, 0.0f), vec3(0.0f), vec3(5.0f));*/
    /*FbxLoader::load(util::prependExecutableDirectory("content/models/sphere.fbx"), polys);

    for (int z = -1; z <= 1; z++) {
        for (int x = -1; x <= 1; x++) {
            transformed.clear();
            transform_mesh(polys, transformed,
                vec3(x * 5.0f, 1.0f, z * 5.0f), vec3(0.0f), vec3(1.0f));
            for (auto & tri : transformed)
                scene->addPoly(tri, bunny);
        }
    }*/

    FbxLoader::load(util::prependExecutableDirectory("content/models/dragon.fbx"), polys);

	for (int z = 0; z <= 0; z++) {
		for (int x = 0; x <= 0; x++) {
			transformed.clear();
			transform_mesh(polys, transformed,
				vec3(x * 5.0f, 1.0f, z * 5.0f), vec3(0.0f), vec3(0.75f));
			for (auto & tri : transformed)
				scene->addPoly(tri, bunny);
		}
	}
    
    polys.clear();
    transformed.clear();

    FbxLoader::load(util::prependExecutableDirectory("content/models/plane.fbx"), polys);
    transform_mesh(polys, transformed, vec3(0, 0, 0), vec3(0, 0, 0), vec3(3, 1, 3));

    for (auto & tri : transformed)
        scene->addPoly(tri, ground);

    Light *light1 = new PointLight(vec3(-20, 20, -20), vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light1);

    Light *light2 = new PointLight(vec3(20, 20, 20), vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light2);

    printf("%lu polygons, %lu lights\n", scene->triangles.size(), scene->lights.size());

    rt = new Raytracer(settings, scene);

    disp = std::make_shared<GLImageDisplay>(1920, 1080, output);

    printf("Rendering\n");

    timer.reset();
    rt->render();

    rt->shutdown(true);

    // TODO: Move into raytracer itself
    float elapsed = (float)timer.getElapsedMilliseconds() / 1000.0f;
    float cpu     = (float)timer.getCPUTime() / 1000.0f;

    printf("Done: %f seconds (total), %f seconds (CPU), speedup: %.02f\n",
        elapsed, cpu, cpu / elapsed);

	printf("Press any key to exit...\n");
	getchar();
    
    return 0;
}
