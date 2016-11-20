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
#include <vector>

namespace MeshLoader {

/**
 * @brief Load polygons from a mesh file
 *
 * @param filename  File to load
 * @param polys     Vector to which polygons will be added
 * @param transform Transformation matrix to apply to vertices
 */
RT_EXPORT void load(std::string filename, std::vector<Triangle> & polys);

};

#endif
