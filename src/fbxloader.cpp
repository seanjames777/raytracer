/**
 * @file fbxloader.cpp
 *
 * @author Sean James
 */

#include <fbxloader.h>
#include <fbxsdk.h>
#include <cassert>
#include <iostream>

void addAttribute(std::vector<Vertex> & vertices, FbxNodeAttribute *attribute, mat4x4 transform,
    mat4x4 transformInverseTranspose)
{
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
                    int normIdx = mesh->GetTextureUVIndex(i, j);
                    vertexUV = uvs->GetDirectArray().GetAt(normIdx);
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

            vec4 position((float)vertexPos[0], (float)vertexPos[1], (float)vertexPos[2], 1.0f);
            position = transform * position; // TODO w / ?

            vec4 normal((float)vertexNorm[0], (float)vertexNorm[1], (float)vertexNorm[2], 0.0f);
            normal = transformInverseTranspose * normal;

            Vertex vertex;
            vertex.position = position.xyz();
            vertex.normal = normalize(normal.xyz());
            vertex.uv = vec2((float)vertexUV[0], (float)vertexUV[1]);
            vertex.color = vec4((float)vertexColor[0], (float)vertexColor[1], (float)vertexColor[2], (float)vertexColor[3]);

            vertices.push_back(vertex);
        }
    }
}

void addNode(std::vector<Vertex> & vertices, FbxNode *node, mat4x4 transform,
    mat4x4 transformInverseTranspose) {
    for (int i = 0; i < node->GetNodeAttributeCount(); i++)
        addAttribute(vertices, node->GetNodeAttributeByIndex(i), transform,
            transformInverseTranspose);

    for (int i = 0; i < node->GetChildCount(); i++)
        addNode(vertices, node->GetChild(i), transform, transformInverseTranspose);
}

void FbxLoader::load(std::string filename, std::vector<Polygon> & polys, mat4x4 transform) {
    FbxManager *fbxManager = FbxManager::Create();
    FbxIOSettings *ioSettings = FbxIOSettings::Create(fbxManager, filename.c_str());
    fbxManager->SetIOSettings(ioSettings);

    FbxImporter *importer = FbxImporter::Create(fbxManager, "");

    assert(importer->Initialize(filename.c_str(), -1, fbxManager->GetIOSettings()));

    FbxScene *scene = FbxScene::Create(fbxManager, "scene");
    importer->Import(scene);
    importer->Destroy();

    FbxNode *rootNode = scene->GetRootNode();

    std::vector<Vertex> vertices;

    mat4x4 transformInverseTranspose = transpose(inverse(transform));

    if (rootNode != NULL)
        for (int i = 0; i < rootNode->GetChildCount(); i++)
            addNode(vertices, rootNode->GetChild(i), transform, transformInverseTranspose);

    fbxManager->Destroy();

    for (int i = 0; i < vertices.size(); i += 3)
        polys.push_back(Polygon(
            vertices[i + 0],
            vertices[i + 1],
            vertices[i + 2]));
}

void FbxLoader::load(std::string filename, std::vector<Polygon> & polys, vec3 translation_v,
    vec3 rotation_v, vec3 scale_v)
{
    mat4x4 transform = scale(scale_v.x, scale_v.y, scale_v.z) *
        yawPitchRoll(rotation_v.x, rotation_v.y, rotation_v.z) *
        translation(translation_v.x, translation_v.y, translation_v.z);

    return load(filename, polys, transform);
}
