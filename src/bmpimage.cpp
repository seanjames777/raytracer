/**
 * @file image.cpp
 *
 * @author Sean James
 */

#include <bmpimage.h>
#include <fstream>
#include <iostream>

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

namespace BMPImage {

S_BITMAPFILEHEADER createBitmapFileHeader(int width, int height) {
    S_BITMAPFILEHEADER fileHeader;

    int pixelsPerRow = width * 3;
    pixelsPerRow = pixelsPerRow % 4 != 0 ? pixelsPerRow + 4 - (pixelsPerRow % 4) : pixelsPerRow;

    fileHeader.signature = 0x4D42;
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;
    fileHeader.pixelArrayOffset = sizeof(S_BITMAPFILEHEADER) + sizeof(S_BITMAPINFOHEADER);
    fileHeader.fileSize = fileHeader.pixelArrayOffset + (pixelsPerRow * height);

    return fileHeader;
}

S_BITMAPINFOHEADER createBitmapInfoHeader(int width, int height) {
    S_BITMAPINFOHEADER infoHeader;

    int pixelsPerRow = width * 3;
    pixelsPerRow = pixelsPerRow % 4 != 0 ? pixelsPerRow + 4 - (pixelsPerRow % 4) : pixelsPerRow;

    infoHeader.headerSize = sizeof(S_BITMAPINFOHEADER);
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = (short)(3 * 8);
    infoHeader.compression = BI_RGB;
    infoHeader.imageSize = (pixelsPerRow * height);
    infoHeader.hres = 2834;
    infoHeader.vres = 2834;
    infoHeader.numColors = 0;
    infoHeader.numImportant = 0;

    return infoHeader;
}

std::shared_ptr<Image> loadBMP(std::string filename) {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::binary | std::ios::in);

    if (!in.is_open()) {
        std::cout << "Error opening '" << filename << "'" << std::endl;
        return nullptr;
    }

    S_BITMAPFILEHEADER fileHeader;
    S_BITMAPINFOHEADER infoHeader;

    in.read((char *)&fileHeader, sizeof(S_BITMAPFILEHEADER));
    in.read((char *)&infoHeader, sizeof(S_BITMAPINFOHEADER));

    int width = infoHeader.width;
    int height = infoHeader.height;
    int channels = infoHeader.bitsPerPixel / 8;

    int pixelsPerRow = width * channels;
    pixelsPerRow = pixelsPerRow % 4 != 0 ? pixelsPerRow + 4 - (pixelsPerRow % 4) : pixelsPerRow;

    in.seekg(fileHeader.pixelArrayOffset, in.beg);

    unsigned char *readPixels = new unsigned char[infoHeader.imageSize];

    in.read((char *)readPixels, infoHeader.imageSize);

    in.close();

    std::shared_ptr<Image> bmp = std::make_shared<Image>(width, height);
    float *arr = new float[width * height * 4];

    for (int y = 0; y < height; y++) {
        int i0 = y * pixelsPerRow;
        int o0 = y * width * 4;

        for (int x = 0; x < width; x++) {
            int i = i0 + x * channels;
            int o = o0 + x * 4;

            switch (channels) {
            case 1:
                arr[o + 0] = readPixels[i] / 255.0f;
                arr[o + 1] = readPixels[i] / 255.0f;
                arr[o + 2] = readPixels[i] / 255.0f;
                arr[o + 3] = 1.0f;
                break;
            case 3:
                arr[o + 0] = readPixels[i + 2] / 255.0f;
                arr[o + 1] = readPixels[i + 1] / 255.0f;
                arr[o + 2] = readPixels[i + 0] / 255.0f;
                arr[o + 3] = 1.0f;
                break;
            case 4:
                arr[o + 0] = readPixels[i + 2] / 255.0f;
                arr[o + 1] = readPixels[i + 1] / 255.0f;
                arr[o + 2] = readPixels[i + 0] / 255.0f;
                arr[o + 3] = readPixels[i + 3] / 255.0f;

                break;
            }
        }
    }

    // TODO: makes a copy
    bmp->setPixels(arr);

    delete [] arr;
    delete [] readPixels;

    return bmp;
}

bool saveBMP(std::string filename, std::shared_ptr<Image> image) {
    int width = image->getWidth();
    int height = image->getHeight();

    int pixelsPerRow = width * 3;
    int pad = pixelsPerRow % 4 != 0 ? 4 - (pixelsPerRow % 4) : 0;

    char *padArr = new char[pad];
    memset(padArr, 0, pad);

    std::ofstream out;
    out.open(filename.c_str(), std::ios::binary | std::ios::out);

    S_BITMAPFILEHEADER fileHeader = createBitmapFileHeader(width, height);
    S_BITMAPINFOHEADER infoHeader = createBitmapInfoHeader(width, height);

    out.write((char *)&fileHeader, sizeof(S_BITMAPFILEHEADER));
    out.write((char *)&infoHeader, sizeof(S_BITMAPINFOHEADER));

    float *pixels = new float[image->getWidth() * image->getHeight() * 4];
    image->getPixels(pixels);

    for (int y = 0; y < height; y++) {
        int i0 = (height - y - 1) * width * 4;

        for (int x = 0; x < width; x++) {
            int i = i0 + x * 4;

            unsigned char r = (unsigned char)((pixels[i + 0]) * 255.0f);
            unsigned char g = (unsigned char)((pixels[i + 1]) * 255.0f);
            unsigned char b = (unsigned char)((pixels[i + 2]) * 255.0f);

            out.write((char *)&b, 1);
            out.write((char *)&g, 1);
            out.write((char *)&r, 1);
        }

        out.write(padArr, pad);
    }

    out.close();

    delete [] padArr;
    delete [] pixels;

    return true;
}

}
