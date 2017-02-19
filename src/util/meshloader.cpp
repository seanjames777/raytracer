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
// TODO: reverse winding at import time

namespace MeshLoader {

void processSubmesh(aiMesh *mesh, const aiScene *scene, Mesh *loadMesh) {
	auto submesh = new Submesh(mesh->mMaterialIndex);

	std::vector<PVVertex> & vertices = submesh->getVertices();
	std::vector<uint32_t> & indices = submesh->getIndices();

	aiVector3D *texCoords = mesh->mTextureCoords[0];

	for (int i = 0; i < mesh->mNumVertices; i++) {
		PVVertex v;

		for (int j = 0; j < 3; j++) {
			v.position[j] = mesh->mVertices[i][j];
			v.normal[j] = mesh->mNormals[i][j];
			v.tangent[j] = mesh->mTangents[i][j];
		}

		if (texCoords) {
			for (int j = 0; j < 2; j++)
				v.uv[j] = texCoords[i][j];
		}
		else {
			for (int j = 0; j < 2; j++)
				v.uv[j] = 0.0f;
		}

		vertices.push_back(v);

		submesh->getBounds().join(float3(v.position[0], v.position[1], v.position[2]));
	}

	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (int j = 0; j < 3; j++)
			indices.push_back(face.mIndices[j]);
	}

	submesh->setName(std::string(mesh->mName.C_Str()));

	loadMesh->addSubmesh(submesh);

	loadMesh->getBounds().join(submesh->getBounds());
}

void processNode(aiNode *node, const aiScene *scene, Mesh *mesh) {
	for (int i = 0; i < node->mNumMeshes; i++)
		processSubmesh(scene->mMeshes[node->mMeshes[i]], scene, mesh);

	for (int i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene, mesh);
}

void processMaterial(aiMaterial *material, Mesh *mesh) {
	MaterialProperties props;

	aiColor3D diffuse;
	material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	//props.diffuseColor = float3(diffuse.r, diffuse.g, diffuse.b);
	props.diffuseColor = 0.6f; // TODO

	props.specularColor = 0.8f; // TODO
	props.specularPower = 8.0f; // TODO
	props.reflectivity = 0.0f; // TODO
	props.transparency = 0.0f;

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

	if (material->GetTextureCount(aiTextureType_OPACITY)) {
		material->GetTexture(aiTextureType_OPACITY, 0, &path);
		props.transparentTexture = std::string(path.C_Str());
	}

	mesh->addMaterial(props);
}

Mesh *load(std::string filename) {
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		return nullptr;

	auto mesh = new Mesh();

	processNode(scene->mRootNode, scene, mesh);

	for (int i = 0; i < scene->mNumMaterials; i++)
		processMaterial(scene->mMaterials[i], mesh);

	return mesh;
}

};
