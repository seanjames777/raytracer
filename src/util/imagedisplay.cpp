/**
 * @file util/imagedisplay.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <util/imagedisplay.h>

#include <cassert>
#include <iostream>
#include <vector>

const char *vs_source =
    "#version 330 core\n"
    "layout(location=0) in vec2 in_position;\n"
    "out vec2 var_uv;\n"
    "void main() {\n"
    "    gl_Position.xy = in_position;\n"
    "    gl_Position.zw = vec2(0.0, 1.0);\n"
    "    var_uv = in_position / 2.0 + 0.5;\n"
    "}\n";

const char *fs_source =
	"#version 330 core\n"
	"in vec2 var_uv;\n"
	"out vec4 out_color;\n"
	"uniform sampler2D tex;\n"
	"uniform vec2 imageSize;\n"
	"void main() {\n"
	"    vec4 image = texture(tex, var_uv);\n"
	"    out_color.rgb = image.rgb;\n"
    "}\n";

// TODO: shorthand for this type
float vertices[] = {
    -1, -1,
    -1,  1,
     1,  1,
    -1, -1,
     1,  1,
     1, -1
};

void glfw_error_callback(int error, const char *msg) {
    printf("GLFW Error (%d): %s\n", error, msg);
}

GLuint compileShaders(const char *vs_source, const char *fs_source) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint shaders = glCreateProgram();

    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);

    GLint result = GL_FALSE;
    int length;

    glGetShaderiv(vs, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &length);
    if (length > 0){
        std::vector<char> message(length+1);
        glGetShaderInfoLog(vs, length, NULL, &message[0]);
        printf("%s\n", &message[0]);
    }

    glShaderSource(ps, 1, &fs_source, NULL);
    glCompileShader(ps);

    glGetShaderiv(ps, GL_COMPILE_STATUS, &result);
    glGetShaderiv(ps, GL_INFO_LOG_LENGTH, &length);
    if (length > 0){
        std::vector<char> message(length+1);
        glGetShaderInfoLog(ps, length, NULL, &message[0]);
        printf("%s\n", &message[0]);
    }

    glAttachShader(shaders, vs);
    glAttachShader(shaders, ps);
    glLinkProgram(shaders);

    glGetProgramiv(shaders, GL_LINK_STATUS, &result);
    glGetProgramiv(shaders, GL_INFO_LOG_LENGTH, &length);
    if (length > 0){
        std::vector<char> message(length+1);
        glGetProgramInfoLog(shaders, length, NULL, &message[0]);
        printf("%s\n", &message[0]);
    }

    glDeleteShader(vs);
    glDeleteShader(ps);

    return shaders;
}

ImageDisplay::ImageDisplay(int width, int height, Image<float, 4> *image)
    : width(width),
      height(height),
      image(image)
{
    // TODO: may be initialized already
    if (!glfwInit()) {
        std::cout << "Error initializing glfw" << std::endl;
        return; // TODO. Also, other error handling doesn't clean up.
    }

    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_SAMPLES, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_SRGB_CAPABLE, true);

    if (!(window = glfwCreateWindow(width, height, "Window", NULL, NULL))) {
        printf("Error opening glfw window\n");
        return;
    }

    pixels = new float[image->getWidth() * image->getHeight() * 4]; // TODO delete

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    // TODO: Init GLEW
    glewExperimental = true;
    int stat = glewInit();

    if (stat != GLEW_OK) {
        std::cout << "Error initialing glew" << std::endl;
        exit(-1);
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // GLEW sometimes produces a spurious error
    glGetError();

    glEnable(GL_FRAMEBUFFER_SRGB);

    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * 6, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, image->getWidth(), image->getHeight(), 0, GL_RGBA,
        GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    shaders = compileShaders(vs_source, fs_source);

    glUseProgram(shaders);
    GLuint textureSamplerLocation = glGetUniformLocation(shaders, "textureSampler");
    GLuint imageSizeLocation = glGetUniformLocation(shaders, "imageSize");
    glUniform1i(textureSamplerLocation, 0);
    glUniform2f(imageSizeLocation, width, height);
    glUseProgram(0);

    GLCHECK();

    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

    GLCHECK();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

ImageDisplay::~ImageDisplay() {
    glfwDestroyWindow(window);

    glfwTerminate();
}

bool ImageDisplay::shouldClose() {
    return glfwWindowShouldClose(window);
}

void ImageDisplay::getViewport(int & x, int & y, int & w, int & h) {
    int windowWidth, windowHeight;
    int imageWidth = image->getWidth();
    int imageHeight = image->getHeight();
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    float imageAspect = (float)imageWidth / (float)imageHeight;

    int viewportHeight = windowHeight;
    int viewportWidth = viewportHeight * imageAspect;

    if (viewportWidth > windowWidth) {
        viewportWidth = windowWidth;
        viewportHeight = viewportWidth / imageAspect;
    }

    x = (windowWidth - viewportWidth) / 2;
    y = (windowHeight - viewportHeight) / 2;
    w = viewportWidth;
    h = viewportHeight;
}

/*
 * Copy the texture to video memory and redraw the image
 * on screen
 */
void ImageDisplay::refresh() {
    image->getPixels(pixels);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(),
        GL_RGBA, GL_FLOAT, pixels);

    glUseProgram(shaders);
    glBindVertexArray(va);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    int x, y, w, h;
    getViewport(x, y, w, h);
	glViewport(x, y, w, h);

    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, w / 2, h);
    glDisable(GL_DEPTH_TEST);

    //glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_SCISSOR_TEST);
    glDisableVertexAttribArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    GLCHECK();
}

void ImageDisplay::swap() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

float2 ImageDisplay::getCursorPos() {
    int x, y, w, h;
    getViewport(x, y, w, h);

    double cx, cy;
    glfwGetCursorPos(window, &cx, &cy);

    cx -= x;
    cy -= y;

    return float2(cx, cy);
}

void ImageDisplay::drawPreviewScene(PVScene *scene) {
    int x, y, w, h;
    getViewport(x, y, w, h);
    scene->draw(x, y, w, h);
}
