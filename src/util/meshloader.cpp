/**
 * @file util/meshloader.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <util/meshloader.h>

#include <cassert>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// TODO: Clean up after assimp?

namespace MeshLoader {

void processSubmesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Mesh> loadMesh) {
	std::shared_ptr<Submesh> submesh = std::make_shared<Submesh>(mesh->mMaterialIndex);

	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		Triangle tri;

		aiVector3D *texCoords = mesh->mTextureCoords[0];

		for (int j = 0; j < 3; j++) {
			int index = face.mIndices[j];
			Vertex v;

			v.position = float3(mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z);
			v.normal = float3(mesh->mNormals[index].x, mesh->mNormals[index].y, mesh->mNormals[index].z);
			v.tangent = float3(mesh->mTangents[index].x, mesh->mTangents[index].y, mesh->mTangents[index].z);

			if (texCoords)
				v.uv = float2(texCoords[index].x, texCoords[index].y);
			else
				v.uv = float2(0.0f, 0.0f);

			tri.v[j] = v;
		}

		submesh->addTriangle(tri);
	}

	loadMesh->addSubmesh(submesh);
}

void processNode(aiNode *node, const aiScene *scene, std::shared_ptr<Mesh> mesh) {
	for (int i = 0; i < node->mNumMeshes; i++)
		processSubmesh(scene->mMeshes[node->mMeshes[i]], scene, mesh);

	for (int i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene, mesh);
}

void processMaterial(aiMaterial *material, std::shared_ptr<Mesh> mesh) {
	MaterialProperties props;

	aiColor3D diffuse;
	material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	//props.diffuseColor = float3(diffuse.r, diffuse.g, diffuse.b);
	props.diffuseColor = 0.6f; // TODO

	props.specularColor = 0.8f; // TODO
	props.specularPower = 8.0f; // TODO
	props.reflectivity = 0.0f; // TODO

	aiString path;

	// TODO: Can have more than one layer
	if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
		material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		props.diffuseTexture = std::string(path.C_Str());
	}

	if (material->GetTextureCount(aiTextureType_NORMALS)) {
		material->GetTexture(aiTextureType_NORMALS, 0, &path);
		props.normalTexture = std::string(path.C_Str());
	}

	if (material->GetTextureCount(aiTextureType_HEIGHT)) {
		material->GetTexture(aiTextureType_HEIGHT, 0, &path);
		props.normalTexture = std::string(path.C_Str());
	}

	if (material->GetTextureCount(aiTextureType_SPECULAR)) {
		material->GetTexture(aiTextureType_SPECULAR, 0, &path);
		props.specularTexture = std::string(path.C_Str());
	}

	if (material->GetTextureCount(aiTextureType_SHININESS)) {
		material->GetTexture(aiTextureType_SHININESS, 0, &path);
		props.specularTexture = std::string(path.C_Str());
	}

	mesh->addMaterial(props);
}

std::shared_ptr<Mesh> load(std::string filename) {
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		return nullptr;

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

	processNode(scene->mRootNode, scene, mesh);

	for (int i = 0; i < scene->mNumMaterials; i++)
		processMaterial(scene->mMaterials[i], mesh);

	return mesh;
}

};
