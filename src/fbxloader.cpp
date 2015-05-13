/**
 * @file fbxloader.cpp
 *
 * @author Sean James
 */

#include <fbxloader.h>
#include <fbxsdk.h>
#include <cassert>
#include <iostream>

void addAttribute(std::vector<Vertex> & vertices, FbxNodeAttribute *attribute) {
    if (attribute->GetAttributeType() != FbxNodeAttribute::eMesh)
        return;

    FbxMesh *mesh = static_cast<FbxMesh *>(attribute);

    FbxVector4 *meshVerts = mesh->GetControlPoints();
    int nVertices = mesh->GetControlPointsCount();

    int nLayers = mesh->GetLayerCount();
    assert(nLayers == 1);

    FbxLayer *layer0 = mesh->GetLayer(0);

    FbxLayerElementNormal *norms = layer0->GetNormals();
    assert(norms->GetMappingMode() == FbxLayerElement::eByControlPoint ||
        norms->GetMappingMode() == FbxLayerElement::eByPolygonVertex);
    assert(norms->GetReferenceMode() == FbxLayerElement::eDirect ||
        norms->GetReferenceMode() == FbxLayerElement::eIndexToDirect);

    FbxLayerElementUV *uvs = layer0->GetUVs();

    if (uvs != NULL) {
        assert(uvs->GetMappingMode() == FbxLayerElement::eByControlPoint ||
            uvs->GetMappingMode() == FbxLayerElement::eByPolygonVertex);
        assert(uvs->GetReferenceMode() == FbxLayerElement::eDirect ||
            uvs->GetReferenceMode() == FbxLayerElement::eIndexToDirect);
    }
    else {
        std::cout << "Warning: Model does not have UVs" << std::endl;
    }

    FbxLayerElementVertexColor *colors = layer0->GetVertexColors();

    if (colors != NULL) {
        assert(colors->GetMappingMode() == FbxLayerElement::eByControlPoint ||
            colors->GetMappingMode() == FbxLayerElement::eByPolygonVertex);
        assert(colors->GetReferenceMode() == FbxLayerElement::eDirect ||
            colors->GetReferenceMode() == FbxLayerElement::eIndexToDirect);
    }
    else {
        std::cout << "Warning: Model does not have vertex colors" << std::endl;
    }

    for (int i = 0; i < mesh->GetPolygonCount(); i++) {
        assert(mesh->GetPolygonSize(i) == 3);

        for (int j = 0; j < 3; j++) {
            int vertIdx = mesh->GetPolygonVertex(i, j);

            FbxVector4 vertexPos = meshVerts[vertIdx];
            FbxVector4 vertexNorm;
            mesh->GetPolygonVertexNormal(i, j, vertexNorm);
            FbxVector2 vertexUV;
            FbxColor vertexColor(1.0, 1.0, 1.0, 1.0);

            if (uvs != NULL) {
                if (uvs->GetMappingMode() == FbxLayerElement::eByControlPoint) {
                    if (uvs->GetReferenceMode() == FbxLayerElement::eDirect)
                        vertexUV = uvs->GetDirectArray().GetAt(vertIdx);
                    else {
                        int idx = uvs->GetIndexArray().GetAt(vertIdx);
                        vertexUV = uvs->GetDirectArray().GetAt(idx);
                    }
                }
                else {
                    int uvIdx = mesh->GetTextureUVIndex(i, j);
                    vertexUV = uvs->GetDirectArray().GetAt(uvIdx);
                }
            }

            if (colors != NULL) {
                if (colors->GetReferenceMode() == FbxLayerElement::eDirect)
                    vertexColor = colors->GetDirectArray().GetAt(vertIdx);
                else {
                    int idx = colors->GetIndexArray().GetAt(vertIdx);
                    vertexColor = colors->GetDirectArray().GetAt(idx);
                }
            }

			vec3 position((float)vertexPos[0], (float)vertexPos[1], (float)vertexPos[2]);
			vec3 normal((float)vertexNorm[0], (float)vertexNorm[1], (float)vertexNorm[2]);

            Vertex vertex;
            vertex.position = position;
            vertex.normal = normalize(normal); // TODO necessary?
            vertex.uv = vec2((float)vertexUV[0], (float)vertexUV[1]);
            vertex.color = vec4((float)vertexColor[0], (float)vertexColor[1], (float)vertexColor[2], (float)vertexColor[3]);

            vertices.push_back(vertex);
        }
    }
}

void addNode(std::vector<Vertex> & vertices, FbxNode *node) {
    for (int i = 0; i < node->GetNodeAttributeCount(); i++)
        addAttribute(vertices, node->GetNodeAttributeByIndex(i));

    for (int i = 0; i < node->GetChildCount(); i++)
        addNode(vertices, node->GetChild(i));
}

void FbxLoader::load(std::string filename, std::vector<Triangle> & polys) {
    FbxManager *fbxManager = FbxManager::Create();
    FbxIOSettings *ioSettings = FbxIOSettings::Create(fbxManager, filename.c_str());
    fbxManager->SetIOSettings(ioSettings);

    FbxImporter *importer = FbxImporter::Create(fbxManager, "");

	bool stat = importer->Initialize(filename.c_str(), -1, fbxManager->GetIOSettings());
    assert(stat);

    FbxScene *scene = FbxScene::Create(fbxManager, "scene");
    importer->Import(scene);
    importer->Destroy();

    FbxNode *rootNode = scene->GetRootNode();

    std::vector<Vertex> vertices;

    if (rootNode != NULL)
        for (int i = 0; i < rootNode->GetChildCount(); i++)
            addNode(vertices, rootNode->GetChild(i));

    fbxManager->Destroy();

    for (size_t i = 0; i < vertices.size(); i += 3)
        polys.push_back(Triangle(
            vertices[i + 0],
            vertices[i + 1],
            vertices[i + 2]));
}
