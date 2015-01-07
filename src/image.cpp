/**
 * @file image.cpp
 *
 * @author Sean James
 */

#include <image.h>
#include <fstream>
#include <iostream>

Image::Image(int width, int height)
    : width(width),
      height(height)
{
    pixels = new float[width * height * 4];

    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            int i0 = (y * width + x) * 4;

            pixels[i0 + 0] = 0.0f;
            pixels[i0 + 1] = 0.0f;
            pixels[i0 + 2] = 0.0f;
            pixels[i0 + 3] = 1.0f;
        }
}

Image::~Image() {
    delete [] pixels;
}

vec4 Image::getPixel(int x, int y) {
    int i = (y * width + x) * 4;

    vec4 color;

    color.x = pixels[i + 0];
    color.y = pixels[i + 1];
    color.z = pixels[i + 2];
    color.w = pixels[i + 3];

    return color;
}

float *Image::getPixels() {
    return pixels;
}

void Image::setPixel(int x, int y, vec4 color) {
    int i = (y * width + x) * 4;

    pixels[i + 0] = color.x;
    pixels[i + 1] = color.y;
    pixels[i + 2] = color.z;
    pixels[i + 3] = color.w;
}

void Image::setPixels(float *data) {
    size_t sz = width * 4 * height * sizeof(float);
    memcpy(this->pixels, data, sz);
}

int Image::getWidth() {
    return width;
}

int Image::getHeight() {
    return height;
}

void Image::applyGamma(float gamma) {
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
}

Sampler::Sampler(FilterMode minFilter, FilterMode magFilter, BorderMode borderU,
    BorderMode borderV)
    : minFilter(minFilter),
      magFilter(magFilter),
      borderU(borderU),
      borderV(borderV)
{
    // TODO make these functions cool
}

vec4 Sampler::sampleBorder(Image *image, int x, int y) {
    int width = image->getWidth();
    int height = image->getHeight();

    // TODO: Because the address mode is fixed, the branch predictor will
    // hopefully not get too confused by the switches.
    // TODO: handle non power of two

    switch(borderU) {
    case Clamp:
        x &= ~(x >> 31);            // Less than 0, fill 0's
        x |= (width - 1 - x) >> 31; // Greater than width - 1, fill 1's
    case Wrap:
        x &= (width - 1);           // Wrap around width - 1
        break;
    }

    switch(borderV) {
    case Clamp:
        y &= ~(y >> 31);
        y |= (height - 1 - y) >> 31;
    case Wrap:
        y &= (height - 1);
        break;
    }

    return image->getPixel(x, y);
}

vec4 Sampler::sample(Image *image, vec2 uv) {
    uv *= 0.8f;

    float x = uv.x * (image->getWidth() - 1);
    float y = uv.y * (image->getHeight() - 1);

    int x0 = (int)x;
    int y0 = (int)y;

    switch(minFilter) {
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

vec4 Sampler::sample(Image *image, vec3 norm) {
    vec2 uv = vec2(atan2f(norm.z, norm.x) + M_PI, acosf(-norm.y));
    uv = uv / vec2(2.0f * M_PI, M_PI);

    return sample(image, uv);
}
