/*
 * Sean James
 *
 * glimagedisplay.h
 *
 * Utility class for displaying an array of pixels in a window
 *
 */

#ifndef _GLIMAGEDISPLAY_H
#define _GLIMAGEDISPLAY_H

#include "defs.h"
#include "quad.h"
#include "bitmap.h"

class GLImageDisplay {
private:

	int width;
	int height;
	int texture;
	int textureChannels;
	char *image;
	Quad quad;

	int internalFormat;
	int format;
	int type;

	void *pixels;

public:

	/*
	 * Default constructor accepts image width and height
	 * and a pointer to the array of pixels
	 */
	GLImageDisplay(int Width, int Height, Bitmap *image);

	/*
	 * Load resources needed to display the image
	 */
	void load();

	/*
	 * Unload resources
	 */
	void unload();

	/*
	 * Copy the texture to video memory and redraw the image
	 * on screen
	 */
	void refresh();

};

#endif
