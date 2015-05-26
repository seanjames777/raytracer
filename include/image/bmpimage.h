/**
 * @file image/image.h
 *
 * @brief Image load/save/processing library
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __BMPIMAGE_H
#define __BMPIMAGE_H

#include <image/image.h>
#include <memory>
#include <rt_defs.h>
#include <string>

namespace BMPImage {

    /**
     * @brief Load a bitmap from a file
     *
     * @param filename BMP file to load
     *
     * @return A pointer to a new bitmap on success, or NULL on error
     */
    RT_EXPORT std::shared_ptr<Image> loadBMP(std::string filename);

    /**
     * @brief Save the image to a .bmp file
     *
     * @param filename BMP file to save into
     *
     * @return True on success, or false on error
     */
    RT_EXPORT bool saveBMP(std::string filename, std::shared_ptr<Image> image);

};

#endif
