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

class FbxLoader {
public:

    /**
     * @brief Load polygons from an FBX file
     *
     * @param filename  File to load
     * @param polys     Vector to which polygons will be added
     * @param transform Transformation matrix to apply to vertices
     */
    static void load(std::string filename, std::vector<Triangle> & polys,
        mat4x4 transform = mat4x4());

    /**
     * @brief Load polygons from an FBX file
     *
     * @param filename    File to load
     * @param polys       Vector to which polygons will be added
     * @param translation Translation to apply to vertices
     * @param rotation    Rotation to apply to vertices
     * @param scale       Scale to apply to vertices
     */
    static void load(std::string filename, std::vector<Triangle> & polys,
        vec3 translation, vec3 rotation, vec3 scale);

};

#endif
