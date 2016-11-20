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

namespace MeshLoader {

void processMesh(aiMesh *mesh, const aiScene *scene, std::vector<Triangle> & polys) {
	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		Triangle tri;

		aiVector3D *texCoords = mesh->mTextureCoords[0];

		for (int j = 0; j < 3; j++) {
			int index = face.mIndices[j];
			Vertex v;

			v.position = float3(mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z);
			v.normal = float3(mesh->mNormals[index].x, mesh->mNormals[index].y, mesh->mNormals[index].z);

			if (texCoords)
				v.uv = float2(texCoords[index].x, texCoords[index].y);
			else
				v.uv = float2(0.0f, 0.0f);

			tri.v[j] = v;
		}

		polys.push_back(tri);
	}
}

void processNode(aiNode *node, const aiScene *scene, std::vector<Triangle> & polys) {
	for (int i = 0; i < node->mNumMeshes; i++)
		processMesh(scene->mMeshes[node->mMeshes[i]], scene, polys);

	for (int i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene, polys);
}

void load(std::string filename, std::vector<Triangle> & polys) {
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(filename, aiProcess_Triangulate);

	// TODO
	assert(scene && scene->mFlags != AI_SCENE_FLAGS_INCOMPLETE && scene->mRootNode);

	processNode(scene->mRootNode, scene, polys);
}

};