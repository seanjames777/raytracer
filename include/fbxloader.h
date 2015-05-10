/**
 * @file fbxloader.h
 *
 * @brief Utility for loading polygons from an FBX file
 *
 * @author Sean James
 */

#ifndef _FBXLOADER_H
#define _FBXLOADER_H

#include <polygon.h>
#include <string>
#include <vector>

class RT_EXPORT FbxLoader {
public:

    /**
     * @brief Load polygons from an FBX file
     *
     * @param filename  File to load
     * @param polys     Vector to which polygons will be added
     * @param transform Transformation matrix to apply to vertices
     */
    static void load(std::string filename, std::vector<Triangle> & polys);

};

#endif
