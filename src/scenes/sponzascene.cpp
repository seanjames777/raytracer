#include <scenes/sponzascene.h>
#include <light/pointlight.h>

SponzaScene::SponzaScene() {
    float3 cameraPosition = float3(-10.5f, 3.5f, 0.0f);

    setCamera(new Camera(cameraPosition, float3(0.0f, 2.5f, -1.0f),
        (float)M_PI / 180.0f * 60.0f, 0.25f, 25.42f));

    auto mesh = MeshLoader::load(relToExeDir("content/models/sponza.obj"));
    addMesh(new MeshInstance(mesh, float3(0.0f, 0.0f, 0.0f), float3(0.0f), 0.02f));

    //int dragonMatID = getNumMaterials();

    auto mesh2 = MeshLoader::load(relToExeDir("content/models/dragon.fbx"));
    addMesh(new MeshInstance(mesh2, float3(0.0f, 0.0f, -0.9f), float3(0.0f, -(float)M_PI / 2.0f, 0.0f), 0.47f));

    //Material *mat = getMaterial(dragonMatID);
    //mat->setOpacity(0.10f);

    if (true) {
        addLight(new PointLight(float3(-10.0f, 5.0f, 0.0f), 0.8f, 30.0f, true));
        addLight(new PointLight(float3( -1.0f, 7.0f, 0.0f), 0.8f, 30.0f, true));
        addLight(new PointLight(float3( 10.0f, 5.0f, 0.0f), 0.8f, 30.0f, true));
    }
    else {
        addLight(new PointLight(float3(  0.0f, 5.0f, -10.0f), 0.5f, 40.0f, true));
        addLight(new PointLight(float3(  0.0f, 5.0f,  10.0f), 0.5f, 40.0f, true));
    }

    setEnvironmentColor(float3(1.0f, 1.0f, 1.0f));
}

SponzaScene::~SponzaScene() {
    
}
