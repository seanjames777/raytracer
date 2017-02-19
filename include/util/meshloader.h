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
#include <math/aabb.h>

struct PVVertex {
    float position[3];
    float normal[3];
    float tangent[3];
    float uv[2];
};

class Submesh {
private:

	std::vector<PVVertex> vertices;
	std::vector<uint32_t> indices;

	unsigned int materialID;
	AABB bounds;
	std::string name;

public:

	// TODO
	std::vector<PVVertex> & getVertices() { return vertices; }
	std::vector<uint32_t> & getIndices() { return indices; }

	Submesh(unsigned int materialID)
		: materialID(materialID),
		  bounds(float3(INFINITY, INFINITY, INFINITY), float3(-INFINITY, -INFINITY, -INFINITY))
	{
	}

	~Submesh() {
	}

	unsigned int getMaterialID() const {
		return materialID;
	}

	AABB & getBounds() {
		return bounds;
	}

	const std::string & getName() const {
		return name;
	}

	void setName(const std::string & name) {
		this->name = name;
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
	AABB bounds;

public:

	Mesh()
		: bounds(float3(INFINITY, INFINITY, INFINITY), float3(-INFINITY, -INFINITY, -INFINITY))
	{
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

	AABB & getBounds() {
		return bounds;
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
