/**
 * @file glimagedisplay.h
 *
 * @brief Utility for displaying an image in a window using OpenGL. Used to visualize in-progress
 * render.
 *
 * @author Sean James
 */

#ifndef _GLIMAGEDISPLAY_H
#define _GLIMAGEDISPLAY_H

#include <image.h>
#include <memory>
#include <thread>
#include <GL/glew.h>
#include <glfw/glfw3.h>
#include <rt_defs.h>

/**
 * @brief Convert an OpenGL error code to a string
 */
inline const char *getErrorString(GLenum error) {
    switch(error) {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM: GLenum argument out of range";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE: Numeric argument out of range";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION: Operation illegal in current state";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY: Not enough memory left to execute command";
    }

    return "Unknown error";
}

/**
 * @brief Check for an OpenGL error
 */
#define GLCHECK() {                                                           \
    GLenum error = glGetError();                                              \
    if (error != GL_NO_ERROR) {                                               \
        std::cout << "GL Error check failed:" << std::endl;                   \
        std::cout << "    At: " << __FILE__ << ":" << __LINE__ << std::endl;  \
        std::cout << " Error: " << getErrorString(error) << std::endl;        \
        exit(-1);                                                             \
    }                                                                         \
}

/**
 * @brief OpenGL image display utility
 */
class RT_EXPORT GLImageDisplay {
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
    GLFWwindow *window;

    /** @brief Displayed image */
    std::shared_ptr<Image> image;

    // TODO
    float *pixels;

    bool display;

    std::thread thread;

    void refresh();

    void worker_thread();

public:

    /**
     * @brief Constructor
     *
     * @param width  Width of the display window
     * @param height Height of the display window
     * @param image  Image to display
     */
    GLImageDisplay(int width, int height, std::shared_ptr<Image> image);

    /**
     * @brief Destructor
     */
    ~GLImageDisplay();

};

#endif