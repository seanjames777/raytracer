/**
 * @file image/image.h
 *
 * @brief Image class
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __IMAGE_H
#define __IMAGE_H

#include <rt_defs.h>

#include <util/align.h>
#include <math/vector.h>

// TODO: Per-image tiling
// TODO: Swizzling vs tiling
// TODO: 3D and possibly 1D textures
// TODO: Mipmaps
// TODO: Cubemaps

#define TILEX 32
#define TILEY 32

/**
 * @brief Stores an array of pixels. Pixels are grouped into tiles to improve cache coherence
 * when the image is sampled with spatial coherence, such as when an image is used for texturing or
 * as a framebuffer. Tiles are stored in scanline order, and pixels within a tile are stored in
 * scanline order. Components are stored sequentially. Image memory is aligned to a cache line.
 * If the data type, component count, and tile size allow each tile to be aligned to cache lines,
 * then this avoids read/modify/write cache contention between tiles.
 *
 * @tparam T Component type
 * @tparam C Number of components
 */
template<typename T, unsigned int C>
class RT_EXPORT Image {
private:

    int  width;  //!< Image width
    int  height; //!< Image height
    int  tilesW; //!< Number of horizontal tiles
    int  tilesH; //!< Number of vertical tiles
    T   *data;   //!< Pixel data

    /**
     * @brief Remap pixel X and Y coordinates to a pixel pointer, accounting for tiling,
     * image size, and component count.
     *
     * @param[in] x Pixel X coordinate
     * @param[in] y Pixel Y coordinate
     *
     * @return Pointer to pixel components
     */
    inline T *remap(int x, int y) const {
        // Note: The compiler *should* convert these to ands and shifts. Otherwise,
        // this is pretty inefficient, especially when the tile size is 1x1. Tiles
        // are stored in scanline order, then scanline order within a tile.
        // TODO: Swapping out bits would be faster and allow for arbitrary swizzling.

        int tileX = x / TILEX;
        int tileY = y / TILEY;
        int pX    = x % TILEX;
        int pY    = y % TILEY;

        return &data[((tileY * tilesW + tileX) * TILEX * TILEY + pY * TILEX + pX) * C];
    }

public:

    /**
     * @brief Constructor
     *
     * @param[in] width    Image width
     * @param[in] height   Image height
     */
    Image(int width, int height)
        : width(width),
          height(height),
          tilesW((width  + TILEX - 1) / TILEX),
          tilesH((height + TILEY - 1) / TILEY),
          data((T *)aligned_alloc(sizeof(T) * tilesW * TILEX * tilesH * TILEY * C, CACHE_LINE))
    {
    }

    /**
     * @brief Destructor
     */
    ~Image() {
        _aligned_free(data);
    }

    /**
     * @brief Copy pixels to an array in scanline order
     *
     * @param[out] data Pixel data
     */
    inline void getPixels(T *data) const {
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
                *((vector<T, C> *)&data[(y * width + x) * C]) = getPixel(x, y);
    }

    /**
     * @brief Copy an array of pixels in scanline order into the image
     */
    inline void setPixels(T *data) {
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
                setPixel(x, y, *((vector<T, C> *)&data[(y * width + x) * C]));
    }

    /**
     * @brief Get a pixel's value
     */
    inline vector<T, C> getPixel(int x, int y) const {
        T *p = remap(x, y);
        return *((vector<T, C> *)p);
    }

    /**
     * @brief Set a pixel's value
     */
    inline void setPixel(int x, int y, const vector<T, C> & pixel) {
        T *p = remap(x, y);
        *((vector<T, C> *)p) = pixel;
    }

    /**
     * @brief Get image width
     */
    inline int getWidth() const {
        return width;
    }

    /**
     * @brief Get image height
     */
    inline int getHeight() const {
        return height;
    }

};

#endif
