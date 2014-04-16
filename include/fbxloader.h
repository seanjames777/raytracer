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
     * @param filename  File to load
     * @param polys     Vector to which polygons will be added
     * @param transform Transformation matrix to apply to vertices
     */
    static void load(std::string filename, std::vector<Polygon> & polys,
        Mat4x4 transform = Mat4x4::identity());

    /**
     * @brief Load polygons from an FBX file
     *
     * @param filename    File to load
     * @param polys       Vector to which polygons will be added
     * @param translation Translation to apply to vertices
     * @param rotation    Rotation to apply to vertices
     * @param scale       Scale to apply to vertices
     */
    static void load(std::string filename, std::vector<Polygon> & polys,
        Vec3 translation, Vec3 rotation, Vec3 scale);

};

#endif
