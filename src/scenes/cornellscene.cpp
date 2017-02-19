#include <scenes/cornellscene.h>
#include <light/pointlight.h>

CornellScene::CornellScene() {
#if 0
    float3 cameraPosition = float3(0.0f, 5.0f, -16.0f);

    setCamera(new Camera(cameraPosition, float3(0.0f, 5.0f, 0.0f),
        (float)M_PI / 4.0f, 0.0f, 15.0f));

    auto mesh1 = MeshLoader::load(relToExeDir("content/models/box.fbx"));
    mesh1->getSubmesh(0)->eraseTriangles(4, 2);
    mesh1->getSubmesh(0)->eraseTriangles(6, 4);
    addMesh(new MeshInstance(mesh1, float3(0.0f, 5.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), 10.0f, true));

    auto mesh2 = MeshLoader::load(relToExeDir("content/models/box.fbx"));
    mesh2->getSubmesh(0)->eraseTriangles(0, 8);
    mesh2->getSubmesh(0)->eraseTriangles(0, 2);
    mesh2->getMaterial(0)->diffuseColor = float3(0.95f, 0.05f, 0.05f);
    addMesh(new MeshInstance(mesh2, float3(0.0f, 5.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), 10.0f, true));

    auto mesh3 = MeshLoader::load(relToExeDir("content/models/box.fbx"));
    mesh3->getSubmesh(0)->eraseTriangles(0, 8);
    mesh3->getSubmesh(0)->eraseTriangles(2, 2);
    mesh3->getMaterial(0)->diffuseColor = float3(0.1f, 0.90f, 0.1f);
    addMesh(new MeshInstance(mesh3, float3(0.0f, 5.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), 10.0f, true));

#if 1
    auto mesh4 = MeshLoader::load(relToExeDir("content/models/box.fbx"));
    addMesh(new MeshInstance(mesh4, float3(1.5f, 1.5f, -1.5f), float3(0.0f, (float)M_PI / 180.0f * 20.0f, 0.0f), 3.0f));
    addMesh(new MeshInstance(mesh4, float3(-2.0f, 3.0f, 1.5f), float3(0.0f, (float)M_PI / 180.0f * -20.0f, 0.0f), float3(3.0f, 6.0f, 3.0f)));
#endif

    addLight(new PointLight(float3(0.0f, 9.0f, 0.0f), 0.5f, 
        float3(17.476f, 13.04f, 7.6f) * 2.5f, true));
#endif
}

CornellScene::~CornellScene() {
    
}
