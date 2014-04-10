/**
 * @file fbxloader.h
 *
 * @brief Utility for loading polygons from an FBX file
 *
 * @author Sean James
 */

#ifndef _FBXLOADER_H
#define _FBXLOADER_H

#include <defs.h>
#include <polygon.h>

class FbxLoader {
public:

    /**
     * @brief Load polygons from an FBX file
     *
     * @param filename File to load
     * @param polys    Vector to which polygons will be added
     */
    static void load(std::string filename, std::vector<Polygon> & polys);

};

#endif
