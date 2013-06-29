/*
 * Sean James
 *
 * shaders.h
 *
 * Shader loading utility function
 *
 */

#ifndef _SHADERS_H
#define _SHADERS_H

#include "defs.h"

/*
 * Utility function to load a vertex and pixel shaders from the given files.
 * Returns an OpenGL handle to the shader
 */
int loadShaders(const char * vertex_file_path,const char * fragment_file_path);

#endif
