/**
 * @file util/meshloader.h
 *
 * @brief Utility for loading polygons from a mesh file
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __MESHLOADER_H
#define __MESHLOADER_H

#include <core/triangle.h>
#include <string>
#include <util/vector.h>
#include <memory>
#include <vector>

class Submesh {
private:

	util::vector<Triangle, 16> triangles;
	unsigned int materialID;

public:

	Submesh(unsigned int materialID)
		: materialID(materialID)
	{
	}

	~Submesh() {
	}

	void addTriangle(Triangle tri) {
		triangles.push_back(tri);
	}

	unsigned int getNumTriangles() const {
		return triangles.size();
	}

	Triangle & getTriangle(unsigned int i) {
		return triangles[i];
	}

	unsigned int getMaterialID() const {
		return materialID;
	}
};

struct MaterialProperties {
	float3 diffuseColor;
	float3 specularColor;
	float  specularPower;
	float  reflectivity;
	float  transparency;

	std::string diffuseTexture;
	std::string normalTexture;
	std::string specularTexture;
	std::string transparentTexture;
};

class Mesh {
private:

	std::vector<Submesh *> submeshes;
	util::vector<MaterialProperties, 16> materials;

public:

	Mesh() {
	}

	~Mesh() {
	}

	void addSubmesh(Submesh *submesh) {
		submeshes.push_back(submesh);
	}

	unsigned int getNumSubmeshes() const {
		return submeshes.size();
	}

	Submesh *getSubmesh(unsigned int i) const {
		return submeshes[i];
	}

	void addMaterial(const MaterialProperties & material) {
		materials.push_back(material); // TODO: move?
	}

	unsigned int getNumMaterials() const {
		return materials.size();
	}

	MaterialProperties *getMaterial(unsigned int i) {
		return &materials[i];
	}
};

namespace MeshLoader {

/**
 * @brief Load polygons from a mesh file
 *
 * @param filename  File to load
 * @param polys     Vector to which polygons will be added
 * @param transform Transformation matrix to apply to vertices
 */
RT_EXPORT Mesh *load(std::string filename);

};

#endif
