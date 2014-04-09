/**
 * @file bitmap.cpp
 *
 * @author Sean James
 */

#include <bitmap.h>

S_BITMAPFILEHEADER Bitmap::createBitmapFileHeader() {
	S_BITMAPFILEHEADER fileHeader;

	int pixelsPerRow = width * channels;
	pixelsPerRow = pixelsPerRow % 4 != 0 ? pixelsPerRow + 4 - (pixelsPerRow % 4) : pixelsPerRow;

	fileHeader.signature = 0x4D42;
	fileHeader.reserved1 = 0;
	fileHeader.reserved2 = 0;
	fileHeader.pixelArrayOffset = sizeof(S_BITMAPFILEHEADER) + sizeof(S_BITMAPINFOHEADER);
	fileHeader.fileSize = fileHeader.pixelArrayOffset + (pixelsPerRow * height);

	return fileHeader;
}

S_BITMAPINFOHEADER Bitmap::createBitmapInfoHeader() {
	S_BITMAPINFOHEADER infoHeader;

	int pixelsPerRow = width * channels;
    pixelsPerRow = pixelsPerRow % 4 != 0 ? pixelsPerRow + 4 - (pixelsPerRow % 4) : pixelsPerRow;

    infoHeader.headerSize = sizeof(S_BITMAPINFOHEADER);
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = (short)(channels * 8);
    infoHeader.compression = BI_RGB;
    infoHeader.imageSize = (pixelsPerRow * height);
    infoHeader.hres = 2834;
    infoHeader.vres = 2834;
    infoHeader.numColors = 0;
    infoHeader.numImportant = 0;

	return infoHeader;
}

Bitmap *Bitmap::load(std::string filename) {
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

	Bitmap *bmp = new Bitmap(width, height, channels);
	unsigned char *arr = bmp->getPixels();

	for (int y = 0; y < height; y++) {
		int i0 = y * pixelsPerRow;
		int o0 = y * width * channels;

		for (int x = 0; x < width; x++) {
			int stride = x * channels;

			int i = i0 + stride;
			int o = o0 + stride;

			switch (channels) {
			case 1:
				arr[o] = readPixels[i];
				break;
			case 3:
				arr[o + 0] = readPixels[o + 2];
				arr[o + 1] = readPixels[o + 1];
				arr[o + 2] = readPixels[o + 0];
				break;
			case 4:
				arr[o + 0] = readPixels[o + 2];
				arr[o + 1] = readPixels[o + 1];
				arr[o + 2] = readPixels[o + 0];
				arr[o + 3] = readPixels[o + 3];

				break;
			}
		}
	}

	return bmp;
}

Bitmap::Bitmap(int width, int height, int channels)
	: width(width),
      height(height),
      channels(channels)
{
	pixels = new unsigned char[width * height * channels];
	memset(pixels, 0, width * height * channels * sizeof(unsigned char));
}

Bitmap::~Bitmap() {
	delete [] pixels;
}

/*
 * Save the bitmap to the given file in the .bmp format
 */
bool Bitmap::save(std::string filename) {
	int pixelsPerRow = width * channels;
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
        int i0 = (height - y - 1) * width * channels;

        for (int x = 0; x < width; x++) {
            int i = i0 + x * channels;

            unsigned char *arr = (unsigned char *)pixels;

            switch(channels) {
            case 1:
                out.write((char *)&arr[i + 0], 1);
                out.write((char *)&arr[i + 0], 1);
                out.write((char *)&arr[i + 0], 1);
                break;
            case 3:
                out.write((char *)&arr[i + 2], 1);
                out.write((char *)&arr[i + 1], 1);
                out.write((char *)&arr[i + 0], 1);
                break;
            case 4:
                out.write((char *)&arr[i + 2], 1);
                out.write((char *)&arr[i + 1], 1);
                out.write((char *)&arr[i + 0], 1);
                out.write((char *)&arr[i + 3], 1);
                break;
            }
        }

        out.write(padArr, pad);
    }

    out.close();

    return true;
}

Vec4 Bitmap::getPixel(Vec2 uv) {
	uv.x = uv.x - (int)uv.x;
	uv.y = uv.y - (int)uv.y;
	
	int x = (int)(uv.x * (width - 1));
	int y = (int)(uv.y * (height - 1));

	return getPixel(x, y);
}

Vec4 Bitmap::getPixel(Vec3 norm) {
    Vec2 uv = Vec2(atan2f(norm.z, norm.x) + M_PI, acosf(-norm.y));
    uv = uv / Vec2(2.0f * M_PI, M_PI);

    return getPixel(uv);
}

Vec4 Bitmap::getPixel(int x, int y) {
	int i = (y * width + x) * channels;

	Vec4 color;

	switch(channels) {
	case 1:
		color.x = pixels[i + 0] / 255.0f;
		color.y = pixels[i + 0] / 255.0f;
		color.z = pixels[i + 0] / 255.0f;
		color.w = 1.0f;
		break;
	case 3:
		color.x = pixels[i + 0] / 255.0f;
		color.y = pixels[i + 1] / 255.0f;
		color.z = pixels[i + 2] / 255.0f;
		color.w = 1.0f;
		break;
	case 4:
		color.x = pixels[i + 0] / 255.0f;
		color.y = pixels[i + 1] / 255.0f;
		color.z = pixels[i + 2] / 255.0f;
		color.w = pixels[i + 3] / 255.0f;
		break;
	}

	return color;
}

void Bitmap::getPixel(int x, int y, unsigned char *r, unsigned char *g, unsigned char *b,
		unsigned char *a)
{
	int i = (y * width + x) * channels;

	*r = pixels[i + 0];
	*g = pixels[i + 1];
	*b = pixels[i + 2];
	*a = pixels[i + 3];
}

unsigned char *Bitmap::getPixels() {
    return pixels;
}

void Bitmap::setPixel(int x, int y, Vec4 color) {
	int i = y * width * channels + x * channels;

	switch(channels) {
	case 1:
		pixels[i + 0] = S_F2I(color.x);
		break;
	case 3:
		pixels[i + 0] = S_F2I(color.x);
		pixels[i + 1] = S_F2I(color.y);
		pixels[i + 2] = S_F2I(color.z);
		break;
	case 4:
		pixels[i + 0] = S_F2I(color.x);
		pixels[i + 1] = S_F2I(color.y);
		pixels[i + 2] = S_F2I(color.z);
		pixels[i + 3] = S_F2I(color.w);
		break;
	}
}

void Bitmap::setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b,
		unsigned char a)
{
	int i = y * width * channels + x * channels;

	pixels[i + 0] = r;
	pixels[i + 1] = g;
	pixels[i + 1] = b;
	pixels[i + 1] = a;
}

void Bitmap::setPixels(unsigned char *data) {
	size_t sz = width * channels * height;
	memcpy(this->pixels, data, sz);
}

int Bitmap::getWidth() {
	return width;
}

int Bitmap::getHeight() {
	return height;
}

int Bitmap::getChannels() {
	return channels;
}
