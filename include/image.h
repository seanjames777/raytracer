/**
 * @file image.h
 *
 * @brief Image load/save/processing library
 *
 * @author Sean James
 */

#ifndef _IMAGE_H
#define _IMAGE_H

#include <string>
#include <math/vector.h>
#include <iostream> // TODO
#include <rt_defs.h>

// TODO: inline functions

//#define USE_TILING
#define LOG_TILEW 5
#define LOG_TILEH 5
#define TILEW (1 << LOG_TILEW)
#define TILEH (1 << LOG_TILEH)

/**
 * @brief Stores an array of integral pixels with between 1 and 4 components and can load and store
 * bitmaps in the .bmp format
 */
class RT_EXPORT Image {
private:

    int width;
    int height;

#ifdef USE_TILING
    int tilesW;
    int tilesH;
#endif

    float *pixels;

    inline int remap(int x, int y) {
#ifdef USE_TILING
        int tx = x >> LOG_TILEW;
        int ty = y >> LOG_TILEH;

        int ox = x & (TILEW - 1);
        int oy = y & (TILEH - 1);

        int idx = (((ty * tilesW) + tx) << LOG_TILEW) << LOG_TILEH;
        idx += (oy << LOG_TILEW) + ox;
        idx <<= 2;

        return idx;
#else
        return (y * width + x) << 2;
#endif
    }

public:

    /**
     * @brief Constructor. All images are 32 bit floating point images for now. TODO.
     *
     * @param width    Image width
     * @param height   Image height
     */
    Image(int width, int height);

    /**
     * @brief Destroy the image
     */
    ~Image();

    /**
     * @brief Get a pointer to the array of pixels
     */
    void getPixels(float *pixels);

    /**
     * @brief Set all pixels
     */
    void setPixels(float *data);

    /**
     * @brief Get a pixel by integer coordinate
     */
    inline vec4 getPixel(int x, int y) {
        int i = remap(x, y);
        return *(vec4 *)(&pixels[i]);
    }

    /**
     * @brief Set a pixel
     */
    inline void setPixel(int x, int y, const vec4 & color) {
        int i = remap(x, y);
        *(vec4 *)(&pixels[i]) = color;
    }

    /**
     * @brief Get the width of the image in pixels
     */
    inline int getWidth() {
        return width;
    }

    /**
     * @brief Get the height of the image in pixels
     */
    inline int getHeight() {
        return height;
    }

    /**
     * @brief Apply a gamma correction to the image
     */
    //void applyGamma(float gamma);

    /**
     * @brief Apply tone mapping to the image
     */
    //void applyTonemapping(float exposure);

};

enum FilterMode {
    Nearest,
    Linear,
    MipLinear
};

enum BorderMode {
    Clamp,
    Wrap
};

// TODO: Might be faster to use template filter settings for inlining
struct RT_EXPORT Sampler {
    FilterMode minFilter;
    FilterMode magFilter;
    BorderMode border;

    vec4 sampleBorder(Image *image, int x, int y);

public:

    Sampler(FilterMode minFilter, FilterMode magFilter, BorderMode border);

    vec4 sample(Image *image, const vec2 & uv);

    vec4 sample(Image *image, const vec3 & norm);

};

#endif
