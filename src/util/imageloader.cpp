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

Image<float, 4> *load(std::string filename) {
	int width, height, components;
	unsigned char *pixels = stbi_load(filename.c_str(), &width, &height, &components, 4);

	if (!pixels) {
		std::cout << "Error opening '" << filename << "'" << std::endl;
		return nullptr;
	}

    Image<float, 4> *bmp = new Image<float, 4>(width, height);

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

}
