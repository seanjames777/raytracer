/**
 * @file image/bmpimage.h
 *
 * @brief Utility for loading and saving images in BMP format
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
     * @brief Load an image from a BMP file
     *
     * @param[in] filename BMP file to load
     *
     * @return Loaded bitmap, or null if there is an error
     */
    RT_EXPORT std::shared_ptr<Image<float, 3>> loadBMP(std::string filename);

    /**
     * @brief Save an image to a BMP file
     *
     * @param[in] filename BMP file to save into
     *
     * @return True on success, or false on error
     */
    RT_EXPORT bool saveBMP(std::string filename, std::shared_ptr<Image<float, 3>> image);

};

#endif
