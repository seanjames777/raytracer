/**
 * @file fbxloader.cpp
 *
 * @author Sean James
 */

#include <fbxloader.h>
#include <fbxsdk.h>

void addAttribute(std::vector<Vertex> & vertices, std::vector<int> & indices,
    FbxNodeAttribute *attribute)
{
    if (attribute->GetAttributeType() != FbxNodeAttribute::eMesh)
        return;

    FbxMesh *mesh = static_cast<FbxMesh *>(attribute);

    FbxVector4 *meshVerts = mesh->GetControlPoints();
    int nVertices = mesh->GetControlPointsCount();

    int nLayers = mesh->GetLayerCount();
    ASSERT(nLayers == 1);
    
    FbxLayer *layer0 = mesh->GetLayer(0);

    FbxLayerElementNormal *norms = layer0->GetNormals();
    ASSERT(norms->GetMappingMode() == FbxLayerElement::eByControlPoint);
    ASSERT(norms->GetReferenceMode() == FbxLayerElement::eDirect);

    FbxLayerElementUV *uvs = layer0->GetUVs();

    if (uvs != NULL) {
        ASSERT(uvs->GetMappingMode() == FbxLayerElement::eByControlPoint);
        ASSERT(uvs->GetReferenceMode() == FbxLayerElement::eDirect);
    }

    for (int i = 0; i < nVertices; i++) {
        FbxVector4 vertexPos = meshVerts[i];
        FbxVector4 vertexNorm = norms->GetDirectArray().GetAt(i);

        Vertex modelVertex;
        modelVertex.position = Vec3((float)vertexPos[0], (float)vertexPos[1], (float)vertexPos[2]);
        modelVertex.normal = Vec3((float)vertexNorm[0], (float)vertexNorm[1], (float)vertexNorm[2]);

        if (uvs != NULL) {
            FbxVector2 vertexUV = uvs->GetDirectArray().GetAt(i);
            modelVertex.uv = Vec2((float)vertexUV[0], (float)vertexUV[1]);
        }
        else
            modelVertex.uv = Vec2(0, 0);

        vertices.push_back(modelVertex);
    }

    for (int i = 0; i < mesh->GetPolygonCount(); i++) {
        ASSERT(mesh->GetPolygonSize(i) == 3);

        for (int j = 0; j < 3; j++)
            indices.push_back(mesh->GetPolygonVertex(i, j));
    }
}

void addNode(std::vector<Vertex> & vertices, std::vector<int> & indices, FbxNode *node) {
    for (int i = 0; i < node->GetNodeAttributeCount(); i++) {
        addAttribute(vertices, indices, node->GetNodeAttributeByIndex(i));
    }

    for (int i = 0; i < node->GetChildCount(); i++) {
        addNode(vertices, indices, node->GetChild(i));
    }
}

void FbxLoader::load(std::string filename, std::vector<Poly> & polys) {
    FbxManager *fbxManager = FbxManager::Create();
    FbxIOSettings *ioSettings = FbxIOSettings::Create(fbxManager, filename.c_str());
    fbxManager->SetIOSettings(ioSettings);

    FbxImporter *importer = FbxImporter::Create(fbxManager, "");

    ASSERT(importer->Initialize(filename.c_str(), -1, fbxManager->GetIOSettings()));

    FbxScene *scene = FbxScene::Create(fbxManager, "scene");
    importer->Import(scene);
    importer->Destroy();

    FbxNode *rootNode = scene->GetRootNode();

    std::vector<Vertex> vertices;
    std::vector<int> indices;

    if (rootNode != NULL)
        for (int i = 0; i < rootNode->GetChildCount(); i++)
            addNode(vertices, indices, rootNode->GetChild(i));

    fbxManager->Destroy();

    for (int i = 0; i < indices.size(); i += 3)
        polys.push_back(Poly(
            vertices[indices[i + 0]],
            vertices[indices[i + 1]],
            vertices[indices[i + 2]]));
}
