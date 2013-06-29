/*
 * Sean James
 *
 * bitmap.cpp
 *
 * Bitmap load/save library
 *
 * TODO: correct code for non-RGB bitmaps, and alpha channels
 *
 */

#ifndef _BITMAP_H
#define _BITMAP_H

#include "defs.h"
#include "rtmath.h"
#include "loggers.h"

// May have been defined by windows
#ifndef WINDOWS
#define BI_RGB 0
#else
#include <windows.h>
#endif

#define S_F2I(n) (unsigned char)(CLAMP(0, 255, (n) * 255.0f))

/*
 * Available bitmap formats
 */
typedef enum _BitmapFormats {
	S_R8      = 0,              // Single channel, 1 byte per channel
	S_RGB8    = 1,              // Red, Green, Blue, 1 byte per channel
	S_DEFAULT = 1,              // Platform supported >= S_RGB8
	S_RGBA8   = 2,              // Red, Green, Blue, Alpha, 1 byte per channel
	S_R32F    = 3,              // Single channel, 4 byte float per channel
	S_RGB32F  = 4,              // Red, Green, Blue, 4 byte float per channel
	S_RGBA32F = 5               // Red, Green, Blue, Alpha, 4 byte float per channel
} BITMAPFORMATS;

// Pack the structs
#ifdef WINDOWS
#pragma pack(push, 1)
#endif

/*
 * Stores the file header of a .bmp file
 */
typedef struct PACKED {
    short signature;             // 0x4D42
    int   fileSize;              // Total size of the file
    short reserved1, reserved2;  // Unused
    int   pixelArrayOffset;      // Offset to the pixel data (52 bytes)
} S_BITMAPFILEHEADER;

/*
 * Stores the bitmap header of a .bmp file
 */
typedef struct PACKED {
    int   headerSize;            // Size of this header (40 bytes)
    int   width;                 // Width of the image in pixels
    int   height;                // Height of the image in pixels
    short planes;                // Number of image planes (1)
    short bitsPerPixel;          // # components * 8
    int compression;             // Compression (NONE=0)
    int imageSize;               // Size of the pixel array
    int hres;                    // Horizontal resolution (0 = automatic)
    int vres;                    // Vertical resolution (0 = automatic)
    int numColors;               // Number of colors (0)
    int numImportant;            // Number of important colors (0)
} S_BITMAPINFOHEADER;

#ifdef WINDOWS
#pragma pack(pop)
#endif

/*
 * Stores an array of integral pixels with between 1 and 4 components and can load
 * and store bitmaps in the .bmp format
 */
class Bitmap {
private:

	void *pixels;                  // Array of pixels
	int width;                     // Width of the image in pixels
	int height;                    // Height of the image in pixels
	int channels;                  // Number of channels
	bool floatingPoint;            // Whether this is a floating point texture

	/*
	 * Populate the file header
	 */
	S_BITMAPFILEHEADER createBitmapFileHeader();

	/*
	 * Populate the info header
	 */
	S_BITMAPINFOHEADER createBitmapInfoHeader();

	/*
	 * Load the image from a .bmp file (used by the constructor)
	 */
	void load(string filename);

public:

	/*
	 * Create a new bitmap with the given width, height, and number of channels
	 */
	Bitmap(int Width, int Height, BITMAPFORMATS Format);

	/*
	 * Load a bitmap from the given file in the .bmp format
	 */
	Bitmap(string filename) ;

	/*
	 * Unload the bitmap
	 */
	~Bitmap();

	/*
	 * Save the bitmap to the given file in the .bmp format
	 */
	void save(string filename);

	/*
	 * Get a pointer to the array of pixels
	 */
	void *getPixels();

	/*
	 * Get a pixel as a Color by UV coordinate
	 */
	Color getPixel(const Vec2 & uv);

	/*
	 * Get a pixel as a Color by integer coordinate
	 */
	Color getPixel(int x, int y);

	/*
	 * Set a pixel using a color by integer coordinate
	 */
	void setPixel(int x, int y, float color);

	/*
	 * Set a pixel using a color by integer coordinate
	 */
	void setPixel(int x, int y, const Color & color);

	/*
	 * Set a pixel by integer coordinate, specifying each pixel as an
	 * integer between 0 and 255 inclusive
	 */
	void setPixel(int x, int y, unsigned char g);

	/*
	 * Set a pixel by integer coordinate, specifying each pixel as an
	 * integer between 0 and 255 inclusive
	 */
	void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);

	/*
	 * Set a pixel by integer coordinate, specifying each pixel as an
	 * integer between 0 and 255 inclusive
	 */
	void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

	/*
	 * Get the width of the image in pixels
	 */
	int getWidth();

	/*
	 * Get the height of the image in pixels
	 */
	int getHeight();

	/*
	 * Get the width * componenents padded by 4
	 */
	int getRowWidth();

	/*
	 * Get the number of color channels per pixel
	 */
	int getNumChannels();

	/*
	 * Whether this is a floating point texture
	 */
	bool isFloatingPoint();
};

#endif
