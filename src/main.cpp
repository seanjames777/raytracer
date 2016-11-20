/**
 * @file main.cpp
 *
 * @brief Raytracer demo application entry point
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/raytracer.h>
#include <core/scene.h>
#include <light/pointlight.h>
#include <math/matrix.h>
#include <util/imageloader.h>
#include <util/meshloader.h>
#include <util/imagedisplay.h>
#include <util/path.h>
#include <shader/pbrshader.h>
#include <fstream>

Vertex transform_vertex(const Vertex & vertex, const float4x4 & transform,
    const float4x4 & transformInverseTranspose)
{
    float4 position = transform * float4(vertex.position, 1.0f);
    float4 normal = transformInverseTranspose * float4(vertex.normal, 0.0f);

    return Vertex(position.xyz(), normalize(normal.xyz()), vertex.uv);
}

void transform_mesh(const std::vector<Triangle> & src, std::vector<Triangle> & dst,
    const float3 & translation_v, const float3 & rotation_v, const float3 & scale_v)
{
	float4x4 transform =
        translation(translation_v.x, translation_v.y, translation_v.z) *
        yawPitchRoll(rotation_v.y, rotation_v.x, rotation_v.z) *
        scale(scale_v.x, scale_v.y, scale_v.z);

	float4x4 transformInverseTranspose = transpose(inverse(transform));

    for (auto & tri : src) {
        dst.push_back(Triangle(
            transform_vertex(tri.v[0], transform, transformInverseTranspose),
            transform_vertex(tri.v[1], transform, transformInverseTranspose),
            transform_vertex(tri.v[2], transform, transformInverseTranspose),
            0 // Note: Invalid triangle ID
        ));
    }
}

int main(int argc, char *argv[]) {
    printf("Loading scene...\n");

    RaytracerSettings settings;
    settings.width = 1920;
    settings.height = 1080;
    settings.pixelSamples = 1;
    settings.occlusionSamples = 5;
    settings.occlusionDistance = 4.0f;
    settings.shadowSamples = 4;
    settings.numThreads = std::thread::hardware_concurrency() - 1; // TODO

    float aspect = (float)settings.width / (float)settings.height;

    // Note: Using shared pointer so that these things are all automatically
    // cleaned up at the end of the program. The raytracing code uses raw
    // pointers to minimize overhead.

    auto camera = std::make_shared<Camera>(float3(10.0f, 3.0f, 0.0f), float3(0.0f, 3.0f, 0.0f),
        aspect, (float)M_PI / 2.0f, 19.25f, 0.0f);

    auto output = std::make_shared<Image<float, 3>>(settings.width, settings.height);

    auto environment = ImageLoader::load(relToExeDir("content/textures/cubemap.bmp"));

    auto checker = ImageLoader::load(relToExeDir("content/textures/checker.bmp"));

    auto check_sampler = std::make_shared<Sampler>(Bilinear, Wrap);

    auto env_sampler = std::make_shared<Sampler>(Nearest, Wrap);

    auto scene = std::make_shared<Scene>(camera.get(), output.get(), env_sampler.get(),
        environment.get());

    auto shader = std::make_shared<PBRShader>();

    std::vector<Triangle> polys, transformed;

    std::ifstream cache("cache.bin", std::ios::in | std::ios::binary);

    if (cache) {
        size_t count;
        cache.read((char *)&count, sizeof(size_t));
        polys.resize(count);
        cache.read((char *)&polys[0], sizeof(Triangle) * count);
        cache.close();
    }
    else {
        printf("Loading Mesh\n");
        MeshLoader::load(relToExeDir("content/models/sponza.obj"), polys);

        std::ofstream ocache("cache.bin", std::ios::out | std::ios::binary);

        size_t size = polys.size();
        ocache.write((char *)&size, sizeof(size_t));
        ocache.write((char *)&polys[0], sizeof(Triangle) * size);

        ocache.close();
    }

    for (int z = 0; z <= 0; z++) {
        for (int x = 0; x <= 0; x++) {
            transformed.clear();
            transform_mesh(polys, transformed,
                float3(x * 5.0f, 0.0f, z * 5.0f), float3(0.0f), float3(0.02f));
            for (auto & tri : transformed)
                scene->addPoly(tri, shader.get());
        }
    }

    polys.clear();
    transformed.clear();

    MeshLoader::load(relToExeDir("content/models/plane.fbx"), polys);
    transform_mesh(polys, transformed, float3(5, 1, -5), float3((float)M_PI / 2.0f, 0, 0), float3(0.25f, 1, 0.25f));

    //for (auto & tri : transformed)
    //    scene->addPoly(tri, shader.get());

    auto light1 = std::make_shared<PointLight>(
        float3(-20, 20, -20), float3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light1.get());

    auto light2 = std::make_shared<PointLight>(
        float3(20, 20, 20), float3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
    scene->addLight(light2.get());

    printf("%lu polygons, %lu lights\n", scene->getTriangles().size(), scene->getLights().size());

    auto rt = std::make_shared<Raytracer>(settings, scene.get());
	auto disp = std::make_shared<ImageDisplay>(1920, 1080, output.get());

    printf("Rendering\n");

    Timer timer;
    timer.reset();

    rt->render();

    while (!rt->finished()) {
        disp->refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    rt->shutdown(true);
    disp->refresh();

    // TODO: Move into raytracer itself
    float elapsed = (float)timer.getElapsedMilliseconds() / 1000.0f;
    float cpu     = (float)timer.getCPUTime() / 1000.0f;

    printf("Done: %f seconds (total), %f seconds (CPU), speedup: %.02f\n",
        elapsed, cpu, cpu / elapsed);

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}
