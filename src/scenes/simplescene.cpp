#include <scenes/simplescene.h>
#include <light/pointlight.h>

SimpleScene::SimpleScene() {
    float3 cameraPosition = float3(5.0f, 3.0f, 0.0f);
    float3 spherePosition = float3(0.0f, 1.0f, 0.0f);

    setCamera(new Camera(cameraPosition, float3(0.0f, 0.0f, 0.0f),
        (float)M_PI / 2.0f, 0.2f, length(spherePosition - cameraPosition)));

    auto mesh1 = MeshLoader::load(relToExeDir("content/models/sphere.fbx"));
    addMesh(mesh1, spherePosition, float3(0.0f), 1.0f);

    auto mesh2 = MeshLoader::load(relToExeDir("content/models/plane.fbx"));
    addMesh(mesh2, float3(0.0f, 0.0f, 0.0f), float3(0.0f), 1.0f);

    addLight(new PointLight(float3(5.0f, 5.0f, 5.0f), 0.3f, 20.0f, true));
}

SimpleScene::~SimpleScene() {
    
}
