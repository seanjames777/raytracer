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

// TODO: inline functions

/**
 * @brief Stores an array of integral pixels with between 1 and 4 components and can load and store
 * bitmaps in the .bmp format
 */
class Image {
private:

    /** @brief Array of pixels */
    float *pixels;

    /** @brief Image width */
    int width;

    /** @brief Image height */
    int height;

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
    float *getPixels();

    /**
     * @brief Get a pixel by integer coordinate
     */
    vec4 getPixel(int x, int y);

    /**
     * @brief Set a pixel
     */
    void setPixel(int x, int y, vec4 color);

    /**
     * @brief Set all pixels
     */
    void setPixels(float *data);

    /**
     * @brief Get the width of the image in pixels
     */
    int getWidth();

    /**
     * @brief Get the height of the image in pixels
     */
    int getHeight();

    /**
     * @brief Apply a gamma correction to the image
     */
    void applyGamma(float gamma);

    /**
     * @brief Apply tone mapping to the image
     */
    void applyTonemapping(float exposure);

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

    vec4 sample(Image *image, vec2 uv);

    vec4 sample(Image *image, vec3 norm);

};

#endif
