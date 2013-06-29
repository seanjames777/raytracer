/*
 * Sean James
 *
 * quad.cpp
 *
 * OpenGL Quad-rendering class
 */

#include "quad.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glfw.h>

/*
 * Load resources needed to render a quad
 */
void Quad::load() {
	GLuint vertexArray;
	texChannels = 3;

	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	float vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	float uvs[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	glGenBuffers(1, (GLuint *)&vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, (GLuint *)&uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, uvs, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	shader = loadShaders("vertex.vs", "pixel.ps");
}

/*
 * Unload resources
 */
void Quad::unload() {
	// TODO
}

/*
 * Draw the quad
 */
void Quad::draw() {
	glUseProgram(shader);

	int texChannelsI = glGetUniformLocation(shader, "TextureChannels");
	glUniform1i(texChannelsI, texChannels);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, NULL);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glUseProgram(0);
}

/*
 * Set the number of display channels if a texture is being drawn
 * on the quad
 */
void Quad::setTextureChannels(int channels) {
	texChannels = channels;
}