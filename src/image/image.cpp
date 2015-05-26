/**
 * @file image/image.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <image/image.h>

#include <fstream>
#include <iostream>
#include <string.h>

Image::Image(int width, int height)
    : width(width),
      height(height)
{
#ifdef USE_TILING
    tilesW = (width + TILEW - 1) / TILEW;
    tilesH = (height + TILEH - 1) / TILEH;

    pixels = new float[tilesW * TILEW * tilesH * TILEH * 4];
    memset(pixels, 0, sizeof(float) * tilesW * TILEW * tilesH * TILEH * 4); // TODO maybe set alpha to 1
#else
    pixels = new float[width * height * 4];
    memset(pixels, 0, sizeof(float) * width * height * 4); // TODO maybe set alpha to 1
#endif
}

Image::~Image() {
    delete [] pixels;
}

void Image::setPixels(float *data) {
    int i0_in = 0;

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            int i0_out = remap(x, y);

            for (int i = 0; i < 4; i++)
                pixels[i0_out++] = data[i0_in++];
        }
}

void Image::getPixels(float *data) const {
    int i0_out = 0;

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            int i0_in = remap(x, y);

            for (int i = 0; i < 4; i++)
                data[i0_out++] = pixels[i0_in++];
        }
}

/*void Image::applyGamma(float gamma) {
    for (int y = 0; y < height; y++)
        for (int x = 0 ; x < width; x++) {
            int i = (y * width + x) * 4;

            pixels[i + 0] = powf(pixels[i + 0], gamma);
            pixels[i + 1] = powf(pixels[i + 1], gamma);
            pixels[i + 2] = powf(pixels[i + 2], gamma);
        }
}

void Image::applyTonemapping(float exposure) {
    for (int y = 0; y < height; y++)
        for (int x = 0 ; x < width; x++) {
            int i = (y * width + x) * 4;

            pixels[i + 0] = pixels[i + 0] * exposure;
            pixels[i + 0] /= 1.0f + pixels[i + 0];

            pixels[i + 1] = pixels[i + 1] * exposure;
            pixels[i + 1] /= 1.0f + pixels[i + 1];

            pixels[i + 2] = pixels[i + 2] * exposure;
            pixels[i + 2] /= 1.0f + pixels[i + 2];
        }
}*/

Sampler::Sampler(FilterMode minFilter, FilterMode magFilter, BorderMode border)
    : minFilter(minFilter),
      magFilter(magFilter),
      border(border)
{
    // TODO make these functions cool
}

inline vec4 Sampler::sampleBorder(const Image *image, int x, int y) const {
    int width = image->getWidth();
    int height = image->getHeight();

    // TODO: Because the address mode is fixed, the branch predictor will
    // hopefully not get too confused by the switches.
    // TODO: handle non power of two

    switch(border) {
    case Clamp:
        x &= ~(x >> 31);            // Less than 0, fill 0's
        x |= (width - 1 - x) >> 31; // Greater than width - 1, fill 1's
        y &= ~(y >> 31);
        y |= (height - 1 - y) >> 31;
    case Wrap:
        x &= (width - 1);           // Wrap around width - 1
        y &= (height - 1);
        break;
    }

    return image->getPixel(x, y);
}

vec4 Sampler::sample(const Image *image, const vec2 & uv) const {
    float x = uv.x * (image->getWidth() - 1);
    float y = uv.y * (image->getHeight() - 1);

    int x0 = (int)x;
    int y0 = (int)y;

    switch(minFilter) {
    default:
    case Nearest:
        return sampleBorder(image, x0, y0);
    case Linear:
    case MipLinear:
        // TODO: Might be able to reuse some component of border logic
        // TODO: Might be worth doing a full distance-weighted blend instead of
        // bilinear.

        // These should be nearby in the cache due to tiling
        vec4 s0 = sampleBorder(image, x0,     y0);
        vec4 s1 = sampleBorder(image, x0 + 1, y0);
        vec4 s2 = sampleBorder(image, x0,     y0 + 1);
        vec4 s3 = sampleBorder(image, x0 + 1, y0 + 1);

        float du = x - x0;
        float dv = y - y0;

        vec4 t0 = du * s1 + (1.0f - du) * s0;
        vec4 t1 = du * s3 + (1.0f - du) * s2;

        return dv * t1 + (1.0f - dv) * t0;
    }
}

vec4 Sampler::sample(const Image *image, const vec3 & norm) const {
    // TODO: This atan2 and acosf is super expensive
    vec2 uv = vec2(atan2f(norm.z, norm.x) + (float)M_PI, acosf(-norm.y));
    uv = uv / vec2(2.0f * (float)M_PI, (float)M_PI);

    return sample(image, uv);
}
