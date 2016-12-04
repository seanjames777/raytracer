#include <scenes/simplescene.h>
#include <light/pointlight.h>

SimpleScene::SimpleScene() {
    setCamera(new Camera(float3(5.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 0.0f),
        (float)M_PI / 2.0f));

    auto mesh1 = MeshLoader::load(relToExeDir("content/models/sphere.fbx"));
    addMesh(mesh1, float3(0.0f, 1.0f, 0.0f), float3(0.0f), 1.0f);

    auto mesh2 = MeshLoader::load(relToExeDir("content/models/plane.fbx"));
    addMesh(mesh2, float3(0.0f, 0.0f, 0.0f), float3(0.0f), 1.0f);

    addLight(new PointLight(float3(5.0f, 5.0f, 5.0f), 20.0f, true));
}

SimpleScene::~SimpleScene() {
    
}
