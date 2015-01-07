/**
 * @file image.h
 *
 * @brief Image load/save/processing library
 *
 * @author Sean James
 */

#ifndef _IMAGE_H
#define _IMAGE_H

#include <rtmath.h>
#include <string>
#include <iostream> // TODO

// TODO: inline functions

#define TILEW 4
#define TILEH 4

/**
 * @brief Stores an array of integral pixels with between 1 and 4 components and can load and store
 * bitmaps in the .bmp format
 */
class Image {
private:

    float *pixels;

    int width;
    int height;
    int tilesW;
    int tilesH;

    inline int remap(int x, int y) {
        int tx = x / TILEW;
        int ty = y / TILEH;

        int ox = x % TILEW;
        int oy = y % TILEH;

        int idx = (ty * tilesW + tx) * TILEW * TILEH;
        idx += oy * TILEW + ox;
        idx *= 4;

        return idx;
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

struct Sampler {
    FilterMode minFilter;
    FilterMode magFilter;
    BorderMode borderU;
    BorderMode borderV;

    vec4 sampleBorder(Image *image, int x, int y);

public:

    Sampler(FilterMode minFilter, FilterMode magFilter, BorderMode borderU,
        BorderMode borderV);

    vec4 sample(Image *image, const vec2 & uv);

    vec4 sample(Image *image, const vec3 & norm);

};

#endif
