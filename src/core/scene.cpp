/**
 * @file core/scene.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/scene.h>
#include <math/matrix.h>
#include <materials/pbrmaterial.h>
#include <util/imageloader.h>
#include <iostream>
#include <map>
#include <cassert>

// TODO: Come up with a better workflow

// TODO: Might be better to compact textures to RGB8
std::map<std::string, Image<float, 4> *> textures;

Image<float, 4> *loadTexture(std::string name) {
    std::string filename = relToExeDir("content/textures/" + name);
    std::cout << "Load texture " << filename << std::endl;

    if (textures.find(filename) != textures.end())
        return textures[filename];

    auto image = ImageLoader::load(filename);
    textures[filename] = image;

    return image;
}

Vertex transformVertex(const Vertex & vertex, const float4x4 & transform,
    const float4x4 & transformInverseTranspose)
{
    float4 position = transform * float4(vertex.position, 1.0f);
    float4 normal = transformInverseTranspose * float4(vertex.normal, 0.0f);
    float4 tangent = transformInverseTranspose * float4(vertex.tangent, 0.0f);

    return Vertex(position.xyz(), normalize(normal.xyz()), normalize(tangent.xyz()), vertex.uv);
}

void Scene::addMesh(Mesh *mesh,
    const float3 & translation,
    const float3 & rotation,
    float          scale)
{
    float4x4 transform =
        ::translation(translation.x, translation.y, translation.z) *
        ::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
        ::scale(scale, scale, scale);

    float4x4 transformInverseTranspose = transpose(inverse(transform));

    unsigned int materialOffset = getNumMaterials();

    for (int i = 0; i < mesh->getNumSubmeshes(); i++) {
        auto submesh = mesh->getSubmesh(i);

        for (int j = 0; j < submesh->getNumTriangles(); j++) {
            const Triangle & tri = submesh->getTriangle(j);

            Triangle transformed(
                transformVertex(tri.v[0], transform, transformInverseTranspose),
                transformVertex(tri.v[1], transform, transformInverseTranspose),
                transformVertex(tri.v[2], transform, transformInverseTranspose),
                triangles.size(),
                submesh->getMaterialID() + materialOffset
            );

            triangles.push_back(transformed);
        }
    }

    for (int i = 0; i < mesh->getNumMaterials(); i++) {
        const MaterialProperties & props = *mesh->getMaterial(i);

        auto material = new PBRMaterial(); // TODO: delete

        material->setDiffuseColor(props.diffuseColor);
        material->setSpecularColor(props.specularColor);
        material->setSpecularPower(props.specularPower);
        material->setReflectivity(props.reflectivity);

        if (props.diffuseTexture != "") {
            auto texture = loadTexture(props.diffuseTexture);
            assert(texture);

            material->setDiffuseTexture(texture);
        }

        if (props.normalTexture != "") {
            auto texture = loadTexture(props.normalTexture);
            assert(texture);

            material->setNormalTexture(texture);
        }

        if (props.specularTexture != "") {
            auto texture = loadTexture(props.specularTexture);
            assert(texture);

            material->setRoughnessTexture(texture);
        }

        if (props.transparentTexture != "") {
            auto texture = loadTexture(props.transparentTexture);
            assert(texture);

            material->setTransparentTexture(texture);
        }

        materials.push_back(material);
    }
}
