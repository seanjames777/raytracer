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
#include <path.h>
#include <sstream>
#include <bmpimage.h>
#include <string.h>
#include <net/protocol.h>

// Arguments
int port = 7878;

void parseArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-port") == 0)
            port = atoi(argv[++i]);
    }
}

class RTServer : public RTProtocolServer {
private:

    std::shared_ptr<Image> output;
    Raytracer *rt;
    bool started = false;
    Timer timer;

    Vertex transform_vertex(const Vertex & vertex, const mat4x4 & transform,
        const mat4x4 & transformInverseTranspose)
    {
        vec4 position = transform * vec4(vertex.position, 1.0f);
        vec4 normal = transformInverseTranspose * vec4(vertex.normal, 0.0f);

        return Vertex(position.xyz(), normal.xyz(), vertex.uv, vertex.color);
    }

    void transform_mesh(const std::vector<Triangle> & src, std::vector<Triangle> & dst,
        vec3 translation_v, vec3 rotation_v, vec3 scale_v)
    {
        mat4x4 transform =
            scale(scale_v.x, scale_v.y, scale_v.z) *
            yawPitchRoll(rotation_v.y, rotation_v.x, rotation_v.z) *
            translation(translation_v.x, translation_v.y, translation_v.z);

        mat4x4 transformInverseTranspose = transpose(inverse(transform));

        for (auto & tri : src) {
            dst.push_back(Triangle(
                transform_vertex(tri.v1, transform, transformInverseTranspose),
                transform_vertex(tri.v2, transform, transformInverseTranspose),
                transform_vertex(tri.v3, transform, transformInverseTranspose)
            ));
        }
    }

public:

    RTServer() {
        RaytracerSettings settings;
        settings.width = 1920;
        settings.height = 1080;
        settings.pixelSamples = 4;
        settings.occlusionSamples = 4;
        settings.occlusionDistance = 5.0f;
        settings.shadowSamples = 4;

        float aspect = (float)settings.width / (float)settings.height;

        //Camera *camera = new Camera(vec3(-80, 25.0f, -80), vec3(0, 5.0f, 0), aspect,
        //    (float)M_PI / 3.4f, 19.25f, 0.0f);
        Camera *camera = new Camera(vec3(-20.0f, 15.0f, -20.0f), vec3(0.0f, 5.0f, 0.0f), aspect,
            (float)M_PI / 3.4f, 19.25f, 0.0f);

        output = std::make_shared<Image>(settings.width, settings.height);

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

        Material *ground = new PBRMaterial();

        Material *bunny = new PBRMaterial();

        std::vector<Triangle> polys, transformed;

        /*FbxLoader::load(
        PathUtil::prependExecutableDirectory("content/models/box.fbx"),
        polys, vec3(0.0f, 2.5f, 0.0f), vec3(0.0f), vec3(5.0f));*/
        FbxLoader::load(PathUtil::prependExecutableDirectory("content/models/dragon.fbx"), polys);

        for (int z = -0; z <= 0; z++) {
            for (int x = -0; x <= 0; x++) {
                for (int y = -0; y <= 0; y++) {
                    transformed.clear();

                    transform_mesh(polys, transformed,
                        vec3(x * 20.0f, y * 20.0f, z * 20.0f), vec3(0.0f, (float)M_PI, 0.0f), vec3(1.0f));

                    for (auto & tri : transformed)
                        scene->addPoly(tri, bunny);
                }
            }
        }

        polys.clear();
        transformed.clear();

        FbxLoader::load(PathUtil::prependExecutableDirectory("content/models/plane.fbx"), polys);
        transform_mesh(polys, transformed, vec3(0, 0, 0), vec3(0, 0, 0), vec3(3, 1, 3));

        for (auto & tri : transformed)
            scene->addPoly(tri, ground);

        Light *light1 = new PointLight(vec3(-20, 20, -20), vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
        scene->addLight(light1);

        Light *light2 = new PointLight(vec3(20, 20, 20), vec3(0.5f, 0.5f, 0.5f), 0.25f, 50.0f, 0.15f, true);
        scene->addLight(light2);

        printf("%lu polygons, %lu lights\n", scene->triangles.size(), scene->lights.size());

        rt = new Raytracer(settings, scene);
    }

    ~RTServer() {
    }

protected:

    virtual SERVER_STATUS handleGetStatus() override {
        if (!started)
            return WAITING;
        else if (!rt->finished())
            return WORKING;
        else
            return FINISHED;
    }

    virtual void handleBeginRender() override {
        printf("Rendering\n");

        rt->render();
        started = true;

        // TODO not terribly accurate
        timer.reset();
    }

    virtual void handleShutdown() override {
        rt->shutdown();

        // TODO: Move into raytracer itself
        float elapsed = timer.getElapsedMilliseconds() / 1000.0;
        float cpu     = timer.getCPUTime() / 1000.0;

        printf("Done: %f seconds (total), %f seconds (CPU), speedup: %.02f\n",
            elapsed, cpu, cpu / elapsed);
    }

    virtual std::shared_ptr<Image> getImage() override {
        return output;
    }

};

RTServer worker;

/*void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("Received SIGINT, shutting down...\n");
        worker.setShouldShutDown();
    }

    // TODO: a little race conditioney/threading weirdness
}*/

int main(int argc, char *argv[]) {
    parseArgs(argc, argv);

    /*if (signal(SIGINT, sig_handler) < 0) {
        printf("Error setting up signal handler\n");
        return -1;
    }*/

    worker.serve(port);

    return 0;
}
