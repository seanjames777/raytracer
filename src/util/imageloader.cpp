/**
 * @file util/imageloader.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <iostream>

#include <util/imageloader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ImageLoader {

std::shared_ptr<Image<float, 4>> load(std::string filename) {
	int width, height, components;
	unsigned char *pixels = stbi_load(filename.c_str(), &width, &height, &components, 4);

	if (!pixels) {
		std::cout << "Error opening '" << filename << "'" << std::endl;
		return nullptr;
	}

    std::shared_ptr<Image<float, 4>> bmp = std::make_shared<Image<float, 4>>(width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
			int i = (y * width + x) * 4;

            bmp->setPixel(x, y, float4(
                pixels[i + 0] / 255.0f,
                pixels[i + 1] / 255.0f,
                pixels[i + 2] / 255.0f,
				pixels[i + 3] / 255.0f));
        }
    }

	stbi_image_free(pixels);

    return bmp;
}

bool saveBMP(std::string filename, std::shared_ptr<Image<float, 3>> image) {
#if 0
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

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float3 color = image->getPixel(x, y);
            unsigned char r = (unsigned char)(color.x * 255.0f);
            unsigned char g = (unsigned char)(color.y * 255.0f);
            unsigned char b = (unsigned char)(color.z * 255.0f);

            out.write((char *)&b, 1);
            out.write((char *)&g, 1);
            out.write((char *)&r, 1);
        }

        out.write(padArr, pad);
    }

    out.close();

    delete [] padArr;

    return true;
#endif

	return false;
}

}
