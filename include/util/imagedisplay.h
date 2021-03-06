/**
 * @file util/imagedisplay.h
 *
 * @brief Utility for displaying an image in a window using OpenGL. Used to visualize in-progress
 * render.
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __GLIMAGEDISPLAY_H
#define __GLIMAGEDISPLAY_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <image/image.h>
#include <rt_defs.h>
#include <core/scene.h>
#include <math/matrix.h>
#include <preview/preview.h>

/**
 * @brief OpenGL image display utility
 */
class RT_EXPORT ImageDisplay {
private:

    /** @brief OpenGL texture */
    GLuint texture;

    /** @brief Shader program */
    GLuint shaders;

    /** @brief Vertex buffer */
    GLuint vb;

    /** @brief Vertex array object */
    GLuint va;

    /** @brief Window width */
    int width;

    /** @brief Window height */
    int height;

    /** @brief GLFW window */
    //GLFWwindow *window;

    /** @brief Displayed image */
    Image<float, 4> *image;

    // TODO
    float *pixels;

public:

    /**
     * @brief Constructor
     *
     * @param width  Width of the display window
     * @param height Height of the display window
     * @param image  Image to display
     */
    ImageDisplay(int width, int height, Image<float, 4> *image);

    /**
     * @brief Destructor
     */
    ~ImageDisplay();

    void refresh();

    void swap();

    bool shouldClose();

    float2 getCursorPos();

    void drawPreviewScene(PVScene *scene);

};

#endif
