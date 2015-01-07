/**
 * @file glimagedisplay.cpp
 *
 * @author Sean James
 */

#include <glimagedisplay.h>
#include <iostream>

const char *vs_source =
    "#version 330 core\n"
    "layout(location=0) in vec2 in_position;\n"
    "out vec2 var_uv;\n"
    "void main() {\n"
    "    gl_Position.xy = in_position;\n"
    "    gl_Position.zw = vec2(0.0, 1.0);\n"
    "    var_uv = in_position / 2.0 + 0.5;\n"
    "    var_uv.y = 1.0 - var_uv.y;\n"
    "}\n";

const char *ps_source =
    "#version 330 core\n"
    "in vec2 var_uv;\n"
    "out vec4 out_color;\n"
    "uniform sampler2D textureSampler;\n"
    "void main() {\n"
    "    out_color = texture(textureSampler, var_uv);\n"
    "    out_color.rgb *= 2.0;\n"
    "    out_color.rgb = out_color.rgb / (1.0 + out_color.rgb);\n"
    "    out_color.rgb = pow(out_color.rgb, vec3(1.0 / 2.2));\n"
    "}\n";

vec2 vertices[6] = {
    vec2(-1, -1),
    vec2(-1, 1),
    vec2(1, 1),

    vec2(-1, -1),
    vec2(1, 1),
    vec2(1, -1)
};

GLImageDisplay::GLImageDisplay(int width, int height, std::shared_ptr<Image> image)
    : width(width),
      height(height),
      image(image)
{
    if (!glfwInit()) {
        std::cout << "Error initializing glfw" << std::endl;
        exit(0);
    }

    pixels = new float[image->getWidth() * image->getHeight() * 4]; // TODO delete

    glfwWindowHint(GLFW_SAMPLES, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_FALSE); // Handled manually

    if (!(window = glfwCreateWindow(width, height, "Window", NULL, NULL))) {
        printf("Error opening glfw window\n");
        exit(0);
    }

    glfwMakeContextCurrent(window);

    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * 6, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, image->getWidth(), image->getHeight(), 0, GL_RGBA,
        GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
    shaders = glCreateProgram();

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

    glShaderSource(ps, 1, &ps_source, NULL);
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

    GLCHECK();
}

GLImageDisplay::~GLImageDisplay() {
    glfwTerminate();
}

/*
 * Copy the texture to video memory and redraw the image
 * on screen
 */
void GLImageDisplay::refresh() {
    image->getPixels(pixels);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->getWidth(), image->getHeight(),
        GL_RGBA, GL_FLOAT, pixels);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaders);
    glBindVertexArray(va);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    GLCHECK();

    glfwSwapBuffers(window);
    glfwPollEvents();
}
