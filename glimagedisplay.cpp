/*
 * Sean James
 *
 * glimagedisplay.cpp
 *
 * Utility class for displaying an array of pixels in a window
 *
 */

#include "glimagedisplay.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glfw.h>

/*
 * Default constructor accepts image width and height
 * and a pointer to the array of pixels
 */
GLImageDisplay::GLImageDisplay(int Width, int Height, Bitmap *image) 
	: width(Width),
	  height(Height),
	  pixels(image->getPixels()),
	  textureChannels(image->getNumChannels())
{
	if (!image->isFloatingPoint()) {
		switch(image->getNumChannels()) {
		case 1:
			format = GL_RED;
			internalFormat = GL_R8;
			type = GL_UNSIGNED_BYTE;
			break;
		case 3:
			format = GL_RGB;
			internalFormat = GL_RGB8;
			type = GL_UNSIGNED_BYTE;
			break;
		case 4:
			format = GL_RGBA;
			internalFormat = GL_RGBA8;
			type = GL_UNSIGNED_BYTE;
			break;
		}
	}
	else {
		switch(image->getNumChannels()) {
		case 1:
			format = GL_RED;
			internalFormat = GL_R32F;
			type = GL_FLOAT;
			break;
		case 3:
			format = GL_RGB;
			internalFormat = GL_RGB32F;
			type = GL_FLOAT;
			break;
		case 4:
			format = GL_RGBA;
			internalFormat = GL_RGBA32F;
			type = GL_FLOAT;
			break;
		}
	}
}

/*
 * Load resources needed to display the image
 */
void GLImageDisplay::load() {
	if (!glfwInit()) {
        printf("Error initializing glfw\n");
        glfwTerminate();
        exit(0);
    }

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 1);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (!glfwOpenWindow(width, height, 0, 0, 0, 0, 32, 0, GLFW_WINDOW)) {
        printf("Error opening glfw window\n");
        exit(0);
    }

	glfwSetWindowPos(1900 - width, 0); // TODO
	glfwSetWindowTitle("OpenGL");
    glfwEnable( GLFW_STICKY_KEYS );
	glfwSwapInterval(1);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        printf("Error initializing GLEW\n");
        exit(0);
    }

	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
	glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

	glGenTextures(1, (GLuint *)&texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	quad.load();
	quad.setTextureChannels(textureChannels);
}

/*
 * Unload resources
 */
void GLImageDisplay::unload() {
	glfwTerminate();
}

/*
 * Copy the texture to video memory and redraw the image
 * on screen
 */
void GLImageDisplay::refresh() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, pixels);

	quad.draw();

	glBindTexture(GL_TEXTURE_2D, 0);

	glfwSwapBuffers();
}
