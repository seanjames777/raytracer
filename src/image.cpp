/**
 * @file image.cpp
 *
 * @author Sean James
 */

#include <image.h>

S_BITMAPFILEHEADER Image::createBitmapFileHeader() {
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

S_BITMAPINFOHEADER Image::createBitmapInfoHeader() {
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

Image *Image::loadBMP(std::string filename) {
	std::ifstream in;
	in.open(filename.c_str(), std::ios::binary | std::ios::in);

	if (!in.is_open()) {
		std::cout << "Error opening '" << filename << "'" << std::endl;
		return NULL;
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

	Image *bmp = new Image(width, height);
	float *arr = bmp->getPixels();

	for (int y = 0; y < height; y++) {
		int i0 = y * pixelsPerRow;
		int o0 = y * width * 4;

		for (int x = 0; x < width; x++) {
			int i = i0 + x * channels;
			int o = o0 + x * 4;

			switch (channels) {
			case 1:
				arr[o + 0] = S_I2F(readPixels[i]);
                arr[o + 1] = S_I2F(readPixels[i]);
                arr[o + 2] = S_I2F(readPixels[i]);
                arr[o + 3] = 1.0f;
				break;
			case 3:
				arr[o + 0] = S_I2F(readPixels[i + 2]);
				arr[o + 1] = S_I2F(readPixels[i + 1]);
				arr[o + 2] = S_I2F(readPixels[i + 0]);
                arr[o + 3] = 1.0f;
				break;
			case 4:
				arr[o + 0] = S_I2F(readPixels[i + 2]);
				arr[o + 1] = S_I2F(readPixels[i + 1]);
				arr[o + 2] = S_I2F(readPixels[i + 0]);
				arr[o + 3] = S_I2F(readPixels[i + 3]);

				break;
			}
		}
	}

	return bmp;
}

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

bool Image::saveBMP(std::string filename) {
	int pixelsPerRow = width * 3;
    int pad = pixelsPerRow % 4 != 0 ? 4 - (pixelsPerRow % 4) : 0;

    char *padArr = new char[pad];
    memset(padArr, 0, pad);

    std::ofstream out;
    out.open(filename.c_str(), std::ios::binary | std::ios::out);

    S_BITMAPFILEHEADER fileHeader = createBitmapFileHeader();
    S_BITMAPINFOHEADER infoHeader = createBitmapInfoHeader();

    out.write((char *)&fileHeader, sizeof(S_BITMAPFILEHEADER));
    out.write((char *)&infoHeader, sizeof(S_BITMAPINFOHEADER));

    for (int y = 0; y < height; y++) {
        int i0 = (height - y - 1) * width * 4;

        for (int x = 0; x < width; x++) {
            int i = i0 + x * 4;

            unsigned char r = S_F2I(pixels[i + 0]);
            unsigned char g = S_F2I(pixels[i + 1]);
            unsigned char b = S_F2I(pixels[i + 2]);

            out.write((char *)&b, 1);
            out.write((char *)&g, 1);
            out.write((char *)&r, 1);
        }

        out.write(padArr, pad);
    }

    out.close();

    return true;
}

bool Image::saveEXR(std::string filename) {
    // TODO error checks here and in saveBMP

    Imf::Header header(width, height);
    header.channels().insert("R", Imf::Channel(Imf::FLOAT));
    header.channels().insert("G", Imf::Channel(Imf::FLOAT));
    header.channels().insert("B", Imf::Channel(Imf::FLOAT));
    header.channels().insert("A", Imf::Channel(Imf::FLOAT));

    Imf::OutputFile file(filename.c_str(), header);

    Imf::FrameBuffer frameBuffer;
    frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, (char *)pixels + sizeof(float) * 0,
        sizeof(float) * 4, sizeof(float) * 4 * width));
    frameBuffer.insert("G", Imf::Slice(Imf::FLOAT, (char *)pixels + sizeof(float) * 1,
        sizeof(float) * 4, sizeof(float) * 4 * width));
    frameBuffer.insert("B", Imf::Slice(Imf::FLOAT, (char *)pixels + sizeof(float) * 2,
        sizeof(float) * 4, sizeof(float) * 4 * width));
    frameBuffer.insert("A", Imf::Slice(Imf::FLOAT, (char *)pixels + sizeof(float) * 3,
        sizeof(float) * 4, sizeof(float) * 4 * width));

    file.setFrameBuffer(frameBuffer);
    file.writePixels(height);

    return true;
}

Vec4 Image::getPixel(Vec2 uv) {
	uv.x = uv.x - (int)uv.x;
	uv.y = uv.y - (int)uv.y;
	
	int x = (int)(uv.x * (width - 1));
	int y = (int)(uv.y * (height - 1));

    // TODO: real filtering

	return getPixel(x, y);
}

Vec4 Image::getPixel(Vec3 norm) {
    Vec2 uv = Vec2(atan2f(norm.z, norm.x) + M_PI, acosf(-norm.y));
    uv = uv / Vec2(2.0f * M_PI, M_PI);

    return getPixel(uv);
}

Vec4 Image::getPixel(int x, int y) {
	int i = (y * width + x) * 4;

	Vec4 color;

	color.x = pixels[i + 0];
	color.y = pixels[i + 1];
	color.z = pixels[i + 2];
	color.w = pixels[i + 3];

	return color;
}

float *Image::getPixels() {
    return pixels;
}

void Image::setPixel(int x, int y, Vec4 color) {
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