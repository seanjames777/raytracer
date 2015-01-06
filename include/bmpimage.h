/**
 * @file image.h
 *
 * @brief Image load/save/processing library
 *
 * @author Sean James
 */

#ifndef _BMPIMAGE_H
#define _BMPIMAGE_H

#include <string>
#include <image.h>
#include <memory>

namespace BMPImage {

    /**
     * @brief Load a bitmap from a file
     *
     * @param filename BMP file to load
     *
     * @return A pointer to a new bitmap on success, or NULL on error
     */
    std::shared_ptr<Image> loadBMP(std::string filename);

    /**
     * @brief Save the image to a .bmp file
     *
     * @param filename BMP file to save into
     *
     * @return True on success, or false on error
     */
    bool saveBMP(std::string filename, std::shared_ptr<Image> image);

};

#endif
