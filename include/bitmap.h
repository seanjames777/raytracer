/**
 * @file bitmap.h
 *
 * @brief Bitmap load/save library
 *
 * @author Sean James
 */

#ifndef _BITMAP_H
#define _BITMAP_H

#include <defs.h>
#include <rtmath.h>

#define S_F2I(n) (unsigned char)((n) * 255.0f)
#define BI_RGB   0

#pragma pack(push, 1)

/**
 * @brief Stores the file header of a .bmp file
 */
typedef struct {
    unsigned short signature;             // 0x4D42
    unsigned int   fileSize;              // Total size of the file
    unsigned short reserved1;             // Unused
    unsigned short reserved2;             // Unused
    unsigned int   pixelArrayOffset;      // Offset to the pixel data (56 bytes)
} S_BITMAPFILEHEADER;

/**
 * @brief Stores the bitmap header of a .bmp file
 */
typedef struct {
    unsigned int   headerSize;            // Size of this header (40 bytes)
    unsigned int   width;                 // Width of the image in pixels
    unsigned int   height;                // Height of the image in pixels
    unsigned short planes;                // Number of image planes (1)
    unsigned short bitsPerPixel;          // # components * 8
    unsigned int   compression;           // Compression (NONE=0)
    unsigned int   imageSize;             // Size of the pixel array
    unsigned int   hres;                  // Horizontal resolution (0 = automatic)
    unsigned int   vres;                  // Vertical resolution (0 = automatic)
    unsigned int   numColors;             // Number of colors (0)
    unsigned int   numImportant;          // Number of important colors (0)
} S_BITMAPINFOHEADER;

#pragma pack(pop)

/**
 * @brief Stores an array of integral pixels with between 1 and 4 components and can load and store
 * bitmaps in the .bmp format
 */
class Bitmap {
private:

	/** @brief Array of pixels */
	unsigned char *pixels;

	/** @brief Image width */
	int width;

	/** @brief Image height */
	int height;

	/** @brief Number of channels */
	int channels;

	/**
     * @brief Populate the file header
     */
    S_BITMAPFILEHEADER createBitmapFileHeader();

    /**
     * @brief Populate the info header
     */
    S_BITMAPINFOHEADER createBitmapInfoHeader();

public:

	/**
	 * @brief Constructor that creates a new bitmap
	 *
	 * @param width    Image width
	 * @param height   Image height
	 * @param channels Number of channels
	 */
	Bitmap(int width, int height, int channels);

	/**
	 * @brief Load a bitmap from a file
	 *
	 * @param filename BMP file to load
	 *
	 * @return A pointer to a new bitmap on success, or NULL on error
     */
    static Bitmap *load(std::string filename);

    /**
	 * @brief Destroy the bitmap
	 */
	~Bitmap();

    /**
     * @brief Save the image to a .bmp file
     *
     * @param filename BMP file to save into
     *
     * @return True on success, or false on error
     */
    bool save(std::string filename);

	/**
	 * @brief Get a pointer to the array of pixels
	 */
	unsigned char *getPixels();

	/**
	 * @brief Sample a pixel as by UV coordinate
	 */
	Vec4 getPixel(Vec2 uv);

	/**
	 * @brief Get a pixel by integer coordinate
	 */
	Vec4 getPixel(int x, int y);

	/**
	 * @brief Get a pixel by integer coordinate
	 */
	void getPixel(int x, int y, unsigned char *r, unsigned char *g, unsigned char *b,
		unsigned char *a);

	/**
	 * @brief Set a pixel
	 */
	void setPixel(int x, int y, Vec4 color);

	/**
	 * @brief Set a pixel
	 */
	void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b,
		unsigned char a);

	/**
	 * @brief Set all pixels
	 */
	void setPixels(unsigned char *data);

	/**
	 * @brief Get the width of the image in pixels
	 */
	int getWidth();

	/**
	 * @brief Get the height of the image in pixels
	 */
	int getHeight();

	/**
	 * @brief Get the number of color channels per pixel
	 */
	int getChannels();

};

#endif
