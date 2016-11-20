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
#include <materials/pbrmaterial.h>
#include <fstream>
#include <map>
#include <iostream>

// TODO: Might be better to compact textures to RGB8
std::map<std::string, std::shared_ptr<Image<float, 3>>> textures;

Vertex transformVertex(const Vertex & vertex, const float4x4 & transform,
    const float4x4 & transformInverseTranspose)
{
    float4 position = transform * float4(vertex.position, 1.0f);
    float4 normal = transformInverseTranspose * float4(vertex.normal, 0.0f);

    return Vertex(position.xyz(), normalize(normal.xyz()), vertex.uv);
}

std::shared_ptr<Image<float, 3>> loadTexture(std::string filename) {
	if (textures.find(filename) != textures.end())
		return textures[filename];

	auto image = ImageLoader::load(filename);
	textures[filename] = image;

	return image;
}

void loadMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Scene> scene,
    const float3 & translation_v, const float3 & rotation_v, const float3 & scale_v)
{
	float4x4 transform =
        translation(translation_v.x, translation_v.y, translation_v.z) *
        yawPitchRoll(rotation_v.y, rotation_v.x, rotation_v.z) *
        scale(scale_v.x, scale_v.y, scale_v.z);

	float4x4 transformInverseTranspose = transpose(inverse(transform));

	unsigned int materialOffset = scene->getNumMaterials();

	for (int i = 0; i < mesh->getNumSubmeshes(); i++) {
		auto submesh = mesh->getSubmesh(i);

		for (int j = 0; j < submesh->getNumTriangles(); j++) {
			const Triangle & tri = submesh->getTriangle(j);

			scene->addPoly(Triangle(
				transformVertex(tri.v[0], transform, transformInverseTranspose),
				transformVertex(tri.v[1], transform, transformInverseTranspose),
				transformVertex(tri.v[2], transform, transformInverseTranspose),
				0, // Note: Invalid triangle ID
				submesh->getMaterialID() + materialOffset
			));
		}
	}

	for (int i = 0; i < mesh->getNumMaterials(); i++) {
		const MaterialProperties & props = mesh->getMaterial(i);

		auto material = new PBRMaterial(); // TODO: delete

		if (props.diffuseTexture != "") {
			std::cout << "Load " << props.diffuseTexture << std::endl;
		}

		scene->addMaterial(material);
	}
}

int main(int argc, char *argv[]) {
    printf("Loading scene...\n");

    RaytracerSettings settings;
    settings.width = 1920;
    settings.height = 1080;
    settings.pixelSamples = 2;
    settings.numThreads = std::thread::hardware_concurrency() - 1; // TODO
	settings.maxDepth = 1;

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

    printf("Loading Mesh\n");

	auto mesh = MeshLoader::load(relToExeDir("content/models/sponza.obj"));

    loadMesh(mesh, scene, float3(0.0f, 0.0f, 0.0f), float3(0.0f), float3(0.02f));

	auto light1 = std::make_shared<PointLight>(
        float3(0, 30, 0), 400.0f, true);
    scene->addLight(light1.get());

    printf("%lu polygons, %lu lights\n", scene->getTriangles().size(), scene->getNumLights());

    auto rt = std::make_shared<Raytracer>(settings, scene.get());
	auto disp = std::make_shared<ImageDisplay>(3840, 2160, output.get());

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
