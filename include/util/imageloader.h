/**
 * @file util/imageloader.h
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

namespace ImageLoader {

/**
 * @brief Load an image from an image file
 *
 * @param[in] filename File to load
 *
 * @return Loaded image, or null if there is an error
 */
// TODO: Alpha channel is wasted, but needed for alignment
RT_EXPORT Image<float, 4> *load(std::string filename);

};

#endif
