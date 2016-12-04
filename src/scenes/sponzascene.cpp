#include <scenes/sponzascene.h>
#include <light/pointlight.h>

SponzaScene::SponzaScene() {
    setCamera(new Camera(float3(10.0f, 3.0f, 0.0f), float3(0.0f, 3.0f, 0.0f),
        (float)M_PI / 2.0f));

    auto mesh = MeshLoader::load(relToExeDir("content/models/sponza.obj"));
    addMesh(mesh, float3(0.0f, 0.0f, 0.0f), float3(0.0f), 0.02f);

    for (int x = -1; x <= 1; x++)
        addLight(new PointLight(float3(x * 10.0f, 5.0f, 0.0f), 20.0f, true));
}

SponzaScene::~SponzaScene() {
    
}
