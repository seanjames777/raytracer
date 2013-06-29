/*
 * Sean James
 *
 * bitmap.cpp
 *
 * Bitmap load/save library
 *
 */

#include "bitmap.h"

/*
 * Populate the file header
 */
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

/*
 * Populate the info header
 */
S_BITMAPINFOHEADER Bitmap::createBitmapInfoHeader() {
	S_BITMAPINFOHEADER infoHeader;

	int pixelsPerRow = width * 3;
	pixelsPerRow = pixelsPerRow % 4 != 0 ? pixelsPerRow + 4 - (pixelsPerRow % 4) : pixelsPerRow;

	infoHeader.headerSize = sizeof(S_BITMAPINFOHEADER);
	infoHeader.width = width;
	infoHeader.height = height;
	infoHeader.planes = 1;
	infoHeader.bitsPerPixel = 3 * 8;
	infoHeader.compression = BI_RGB;
	infoHeader.imageSize = (pixelsPerRow * height);
	infoHeader.hres = 2834;
	infoHeader.vres = 2834;
	infoHeader.numColors = 0;
	infoHeader.numImportant = 0;

	return infoHeader;
}

/*
 * Load the image from a .bmp file (used by the constructor)
 */
void Bitmap::load(string filename) {
	std::ifstream in;
	in.open(filename.c_str(), std::ios::binary | std::ios::in);

	if (!in.is_open()) {
		std::cout << "Error opening '" << filename << "'" << std::endl;
		return;
	}

	S_BITMAPFILEHEADER fileHeader;
	S_BITMAPINFOHEADER infoHeader;

	in.read((char *)&fileHeader, sizeof(S_BITMAPFILEHEADER));
	in.read((char *)&infoHeader, sizeof(S_BITMAPINFOHEADER));

	width = infoHeader.width;
	height = infoHeader.height;
	channels = infoHeader.bitsPerPixel / 8;

	int pixelsPerRow = width * channels;
	pixelsPerRow = pixelsPerRow % 4 != 0 ? pixelsPerRow + 4 - (pixelsPerRow % 4) : pixelsPerRow;

	in.seekg(fileHeader.pixelArrayOffset, in.beg);

	unsigned char *readPixels = new unsigned char[infoHeader.imageSize];

	in.read((char *)readPixels, infoHeader.imageSize);

	in.close();

	// TODO
	pixels = new unsigned char[width * height * channels];
	unsigned char *arr = (unsigned char *)pixels;

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

	delete [] readPixels;
}

/*
 * Create a new bitmap with the given width, height, and number of channels
 */
Bitmap::Bitmap(int Width, int Height, BITMAPFORMATS Format) 
	: width(Width), height(Height), pixels(NULL)
{
	switch(Format) {
	case S_R8:
	case S_R32F:
		channels = 1;
		break;
	case S_RGB8:
	case S_RGB32F:
		channels = 3;
		break;
	case S_RGBA8:
	case S_RGBA32F:
		channels = 4;
		break;
	}

	switch (Format) {
	case S_R8:
	case S_RGB8:
	case S_RGBA8:
		floatingPoint = false;
		break;
	case S_R32F:
	case S_RGB32F:
	case S_RGBA32F:
		floatingPoint = true;
		break;
	}

	if (floatingPoint) {
		pixels = new float[width * height * channels];
		memset(pixels, 0, width * height * channels * sizeof(float));
	}
	else {
		pixels = new unsigned char[width * height * channels];
		memset(pixels, 0, width * height * channels * sizeof(unsigned char));
	}
}

/*
 * Load a bitmap from the given file in the .bmp format
 */
Bitmap::Bitmap(string filename) 
	: pixels(NULL)
{
	load(filename);
}

/*
 * Unload the bitmap
 */
Bitmap::~Bitmap() {
	if (pixels != NULL)
		delete [] pixels;
}

/*
 * Save the bitmap to the given file in the .bmp format
 */
void Bitmap::save(string filename) {
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
		int i0 = y * width * channels;

		for (int x = 0; x < width; x++) {
			int i = i0 + x * channels;

			if (!floatingPoint) {
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
					//out.write((char *)&arr[i + 3], 1);
					break;
				}
			}
			else {
				unsigned char r, g, b, a;
				float *arr = (float *)pixels;

				switch(channels) {
				case 1:
					r = S_F2I(arr[i + 0]);

					out.write((char *)&r, 1);
					out.write((char *)&r, 1);
					out.write((char *)&r, 1);
					break;
				case 3:
					r = S_F2I(arr[i + 2]);
					g = S_F2I(arr[i + 1]);
					b = S_F2I(arr[i + 0]);

					out.write((char *)&r, 1);
					out.write((char *)&g, 1);
					out.write((char *)&b, 1);
					break;
				case 4:
					r = S_F2I(arr[i + 2]);
					g = S_F2I(arr[i + 1]);
					b = S_F2I(arr[i + 0]);
					a = S_F2I(arr[i + 3]);

					out.write((char *)&r, 1);
					out.write((char *)&g, 1);
					out.write((char *)&b, 1);
					//out.write((char *)&a, 1);
					break;
				}
			}
		}

		out.write(padArr, pad);
	}

	out.close();

	Loggers::Main.Log(LOGGER_CRITICAL, "Bitmap: written to '%s'\n", filename.c_str());
}

/*
 * Get a pointer to the array of pixels
 */
void *Bitmap::getPixels() {
	return pixels;
}

/*
 * Get a pixel as a Color by UV coordinate
 */
Color Bitmap::getPixel(const Vec2 & uv) {
	Vec2 sample = uv;

	if (sample.x < 0.0f)
		sample.x = 0.5f - sample.x;
	if (sample.y < 0.0f)
		sample.y = 0.5f - sample.y;

	int x = (int)(sample.x * width) % width;
	int y = (int)(sample.y * height) % height;

	return getPixel(x, y);
}

/*
 * Get a pixel as a Color by integer coordinate
 */
Color Bitmap::getPixel(int x, int y) {
	int i = y * width * channels + x * channels;

	Color color;

	if (floatingPoint) {
		float *arr = (float *)pixels;
		float g = (float)arr[i + 0];

		switch(channels) {
		case 1:
			color.r = g;
			color.g = g;
			color.b = g;
			color.a = 1.0f;
			break;
		case 3:
			color.r = g;
			color.g = (float)arr[i + 1];
			color.b = (float)arr[i + 2];
			color.a = 255.0f;
			break;
		case 4:
			color.r = g;
			color.g = (float)arr[i + 1];
			color.b = (float)arr[i + 2];
			color.a = (float)arr[i + 3];
			break;
		}
	}
	else {
		unsigned char *arr = (unsigned char *)pixels;
		float g = arr[i + 0] / 255.0f;

		switch(channels) {
		case 1:
			color.r = g;
			color.g = g;
			color.b = g;
			color.a = 1.0f;
			break;
		case 3:
			color.r = g;
			color.g = arr[i + 1] / 255.0f;
			color.b = arr[i + 2] / 255.0f;
			color.a = 1.0f;
			break;
		case 4:
			color.r = g;
			color.g = arr[i + 1] / 255.0f;
			color.b = arr[i + 2] / 255.0f;
			color.a = arr[i + 3] / 255.0f;
			break;
		}
	}

	return color;
}

/*
 * Set a pixel using a color by integer coordinate
 */
void Bitmap::setPixel(int x, int y, float col) {
	int i = y * width + x;

	if (floatingPoint) {
		float *arr = (float *)pixels;

		switch(channels) {
		case 1:
			arr[i + 0] = col;
			break;
		case 3:
			arr[i + 0] = col;
			arr[i + 1] = col;
			arr[i + 2] = col;
			break;
		case 4:
			arr[i + 0] = col;
			arr[i + 1] = col;
			arr[i + 2] = col;
			arr[i + 3] = 1.0f;
			break;
		}
	}
	else {
		unsigned char *arr = (unsigned char *)pixels;

		unsigned char colI = S_F2I(col);

		switch(channels) {
		case 1:
			arr[i + 0] = colI;
			break;
		case 3:
			arr[i + 0] = colI;
			arr[i + 1] = colI;
			arr[i + 2] = colI;
			break;
		case 4:
			arr[i + 0] = colI;
			arr[i + 1] = colI;
			arr[i + 2] = colI;
			arr[i + 3] = 255;
			break;
		}
	}
}

/*
 * Set a pixel using a color by integer coordinate
 */
void Bitmap::setPixel(int x, int y, const  Color & color) {
	int i = y * width * channels + x * channels;

	Color col = color; // TODO

	if (floatingPoint) {
		float *arr = (float *)pixels;

		switch(channels) {
		case 1:
			arr[i + 0] = col.r;
			break;
		case 3:
			arr[i + 0] = col.r;
			arr[i + 1] = col.g;
			arr[i + 2] = col.b;
			break;
		case 4:
			arr[i + 0] = col.r;
			arr[i + 1] = col.g;
			arr[i + 2] = col.b;
			arr[i + 3] = col.a;
			break;
		}
	}
	else {
		unsigned char *arr = (unsigned char *)pixels;

		switch(channels) {
		case 1:
			arr[i + 0] = S_F2I(col.r);
			break;
		case 3:
			arr[i + 0] = S_F2I(col.r);
			arr[i + 1] = S_F2I(col.g);
			arr[i + 2] = S_F2I(col.b);
			break;
		case 4:
			arr[i + 0] = S_F2I(col.r);
			arr[i + 1] = S_F2I(col.g);
			arr[i + 2] = S_F2I(col.b);
			arr[i + 3] = S_F2I(col.a);
			break;
		}
	}
}

/*
 * Set a pixel using a color by integer coordinate
 */
void Bitmap::setPixel(int x, int y, unsigned char col) {
	int i = y * width + x;

	if (floatingPoint) {
		float *arr = (float *)pixels;

		float colF = col / 255.0f;

		switch(channels) {
		case 1:
			arr[i + 0] = colF;
			break;
		case 3:
			arr[i + 0] = colF;
			arr[i + 1] = colF;
			arr[i + 2] = colF;
			break;
		case 4:
			arr[i + 0] = colF;
			arr[i + 1] = colF;
			arr[i + 2] = colF;
			arr[i + 3] = 1.0f;
			break;
		}
	}
	else {
		unsigned char *arr = (unsigned char *)pixels;

		switch(channels) {
		case 1:
			arr[i + 0] = col;
			break;
		case 3:
			arr[i + 0] = col;
			arr[i + 1] = col;
			arr[i + 2] = col;
			break;
		case 4:
			arr[i + 0] = col;
			arr[i + 1] = col;
			arr[i + 2] = col;
			arr[i + 3] = 255;
			break;
		}
	}
}

/*
 * Set a pixel using a color by integer coordinate
 */
void Bitmap::setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	int i = y * width * 3 + x * 3;

	if (floatingPoint) {
		float *arr = (float *)pixels;

		float colR = (float)r / 255.0f;
		float colG = (float)g / 255.0f;
		float colB = (float)b / 255.0f;

		switch(channels) {
		case 1:
			arr[i + 0] = colR;
			break;
		case 3:
			arr[i + 0] = colR;
			arr[i + 1] = colG;
			arr[i + 2] = colB;
			break;
		case 4:
			arr[i + 0] = colR;
			arr[i + 1] = colG;
			arr[i + 2] = colB;
			arr[i + 3] = 1.0f;
			break;
		}
	}
	else {
		unsigned char *arr = (unsigned char *)pixels;

		switch(channels) {
		case 1:
			arr[i + 0] = r;
			break;
		case 3:
			arr[i + 0] = r;
			arr[i + 1] = g;
			arr[i + 2] = b;
			break;
		case 4:
			arr[i + 0] = r;
			arr[i + 1] = g;
			arr[i + 2] = b;
			arr[i + 3] = 255;
			break;
		}
	}
}

/*
 * Set a pixel using a color by integer coordinate
 */
void Bitmap::setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	int i = y * width * 4 + x * 4;

	if (floatingPoint) {
		float *arr = (float *)pixels;

		float colR = (float)r / 255.0f;
		float colG = (float)g / 255.0f;
		float colB = (float)b / 255.0f;
		float colA = (float)a / 255.0f;

		switch(channels) {
		case 1:
			arr[i + 0] = colR;
			break;
		case 3:
			arr[i + 0] = colR;
			arr[i + 1] = colG;
			arr[i + 2] = colB;
			break;
		case 4:
			arr[i + 0] = colR;
			arr[i + 1] = colG;
			arr[i + 2] = colB;
			arr[i + 3] = colA;
			break;
		}
	}
	else {
		unsigned char *arr = (unsigned char *)pixels;

		switch(channels) {
		case 1:
			arr[i + 0] = r;
			break;
		case 3:
			arr[i + 0] = r;
			arr[i + 1] = g;
			arr[i + 2] = b;
			break;
		case 4:
			arr[i + 0] = r;
			arr[i + 1] = g;
			arr[i + 2] = b;
			arr[i + 3] = a;
			break;
		}
	}
}

/*
 * Get the width of the image in pixels
 */
int Bitmap::getWidth() {
	return width;
}

/*
 * Get the height of the image in pixels
 */
int Bitmap::getHeight() {
	return height;
}

/*
 * Get the width * componenents
 */
int Bitmap::getRowWidth() {
	return width * channels;
}

/*
 * Get the number of color channels per pixel
 */
int Bitmap::getNumChannels() {
	return channels;
}

/*
 * Whether this is a floating point texture
 */
bool Bitmap::isFloatingPoint() {
	return floatingPoint;
}