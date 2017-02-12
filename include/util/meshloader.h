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

class Submesh {
private:

	util::vector<Triangle, 16> triangles;
	unsigned int materialID;
	AABB bounds;
	std::string name;

public:

	// TODO
	util::vector<Triangle, 16> & getTriangles() { return triangles; }

	Submesh(unsigned int materialID)
		: materialID(materialID),
		  bounds(float3(INFINITY, INFINITY, INFINITY), float3(-INFINITY, -INFINITY, -INFINITY))
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

	void eraseTriangles(unsigned int i, unsigned int count) {
		// TODO: vector erase
		util::vector<Triangle, 16> newTriangles;

		for (unsigned int j = 0; j < triangles.size(); j++)
			if (j < i || j >= i + count)
				newTriangles.push_back(triangles[j]);
		
		// TODO: copy assignment
		triangles.clear();

		for (auto & tri : newTriangles)
			triangles.push_back(tri);
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
