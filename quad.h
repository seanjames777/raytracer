/*
 * Sean James
 *
 * quad.h
 *
 * OpenGL Quad-rendering class
 */

#ifndef _QUAD_H
#define _QUAD_H

#define _USE_MATH_DEFINES
#include <math.h>

#include "shaders.h"

/*
 * OpenGL Quad-rendering class
 */
class Quad {
private:

	int shader;       // Shader handle
	int vertexBuffer; // Vertex buffer handle
	int uvBuffer;     // UV buffer handle
	int texChannels;  // Number of texture channels

public:

	/*
	 * Load resources needed to render a quad
	 */
	void load();

	/*
	 * Unload resources
	 */
	void unload();

	/* 
	 * Draw the quad
	 */
	void draw();

	/*
	 * Set the number of display channels if a texture is being drawn
	 * on the quad
	 */
	void setTextureChannels(int channels);
};

#endif
