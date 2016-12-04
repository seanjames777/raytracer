#include <scenes/sponzascene.h>
#include <light/pointlight.h>

SponzaScene::SponzaScene() {
    float3 cameraPosition = float3(-25.5f, 3.5f, 1.0f);

    setCamera(new Camera(cameraPosition, float3(0.0f, 2.5f, -1.0f),
        (float)M_PI / 180.0f * 25.0f, 0.5f, 24.49f));

    auto mesh = MeshLoader::load(relToExeDir("content/models/sponza.obj"));
    addMesh(mesh, float3(0.0f, 0.0f, 0.0f), float3(0.0f), 0.02f);

    auto mesh2 = MeshLoader::load(relToExeDir("content/models/dragon.fbx"));
    addMesh(mesh2, float3(0.0f, 0.0f, -0.9f), float3(0.0f, (float)M_PI / 2.0f, 0.0f), 0.47f);

    if (true) {
        addLight(new PointLight(float3(-10.0f, 5.0f, 0.0f), 0.8f, 20.0f, true));
        addLight(new PointLight(float3(  0.0f, 7.0f, 0.0f), 0.8f, 20.0f, true));
        addLight(new PointLight(float3( 10.0f, 5.0f, 0.0f), 0.8f, 20.0f, true));
    }
    else {
        addLight(new PointLight(float3(  0.0f, 5.0f, -10.0f), 0.5f, 40.0f, true));
        addLight(new PointLight(float3(  0.0f, 5.0f,  10.0f), 0.5f, 40.0f, true));
    }
}

SponzaScene::~SponzaScene() {
    
}
