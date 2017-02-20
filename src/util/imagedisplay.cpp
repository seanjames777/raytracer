/**
 * @file util/imagedisplay.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <util/imagedisplay.h>

#include <util/timer.h>

#include <cassert>
#include <iostream>
#include <vector>
#include <preview/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <preview/imgui_internal.h>
#include <preview/ImGuizmo.h>
#include <math/plane.h>
#include <math/sphere.h>

GLFWwindow *window;
GLuint vao;
GLint viewSizeLocation;
double currTime;

PVShader *shader = nullptr;
PVBuffer *vertices = nullptr;
PVBuffer *indices = nullptr;

PVShader *solidShader = nullptr;
PVBuffer *solidVB = nullptr;
PVVertexArray *solidVA = nullptr;

PVMeshInstance *selectedInstance = nullptr;
PVLight *selectedLight = nullptr;
Camera *selectedCamera = nullptr;

PVSampler *uiSampler = nullptr;
PVTexture *lightIcon = nullptr;
PVFilterMode filterMode = PVFilterModeBilinear;
int anisotropy = 1;
bool lighting = true;
bool texturing = true;
bool normalMapping = true;
int sampleCount = 0;
int maxSampleCount = 0;
bool draggingCamera = false;
bool wireframe = false;

PVTexture *resolveTexture = nullptr;
PVFramebuffer *resolveFramebuffer = nullptr;

PVTexture *colorTexture = nullptr;
PVTexture *depthTexture = nullptr;
PVFramebuffer *framebuffer = nullptr;
PVTexture *shadowTexture = nullptr;
PVFramebuffer *shadowFramebuffer = nullptr;

double mouseX, mouseY;
bool mouseDown;

float camFOV;
bool culling;
bool vsync = true;
int maxLights;
float scroll;

uint64_t frameIdx = 0;

#define MAX_FRAMES 120
float cpuFrameTimes[MAX_FRAMES];
float gpuFrameTimes[MAX_FRAMES];

#define MAX_QUERIES 3
GLuint queries[MAX_QUERIES];

bool draggingTransform = false;
int selectedAxes = -1;
int selectedPlane = -1;
float3 selectedOffset;

enum TransformMode {
    TransformModeTranslation,
    TransformModeRotation,
    TransformModeScale
};

// TODO: Make sure transform mode and local do not change while dragging the tool
// TODO: Camera relative transform mode?
TransformMode transformMode = TransformModeTranslation;
bool local = false;

#if 0
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
#endif

void glfw_error_callback(int error, const char *msg) {
    printf("GLFW Error (%d): %s\n", error, msg);
}

void glfw_mouse_callback(GLFWwindow *window, int button, int action, int modifiers) {
    ImGuiIO & io = ImGui::GetIO();

    if (button >= 0 && button < 3)
        io.MouseDown[button] = action == GLFW_PRESS;
}

void glfw_key_callback(GLFWwindow *window, int key, int foo, int action, int modifiers) {
    ImGuiIO & io = ImGui::GetIO();

    io.KeysDown[key] = action == GLFW_PRESS;

    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

#if 0
const char *get_clipboard(void *data) {
    return glfwGetClipboardString((GLFWwindow *)window);
}

void set_clipboard(void *data, const char *text) {
    glfwSetClipboardString((GLFWwindow *)window, text);
}
#endif

void glfw_scroll_callback(GLFWwindow *window, double x, double y) {
    scroll = y;
}

void glfw_char_callback(GLFWwindow *window, unsigned int c) {
    ImGuiIO & io = ImGui::GetIO();

    if (io.WantCaptureKeyboard) {
        if (c > 0 && c < 0x10000)
            io.AddInputCharacter((unsigned short)c);
    }
    else {
        switch (c) {
        case 'w':
            transformMode = TransformModeTranslation;
            break;
        case 'e':
            transformMode = TransformModeRotation;
            break;
        case 'r':
            transformMode = TransformModeScale;
            break;
        case 'l':
            local = !local;
            break;
        default:
            break;
        }
    }
}

void imgui_render(ImDrawData *data) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGuiIO & io = ImGui::GetIO();

    shader->bind();

    if (viewSizeLocation >= 0)
        glUniform2f(viewSizeLocation, io.DisplaySize.x, io.DisplaySize.y);

    data->ScaleClipRects(io.DisplayFramebufferScale);

    glBindVertexArray(vao);

    glEnable(GL_BLEND);
    glEnable(GL_SCISSOR_TEST);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uiSampler->bind(0);

    for (int i = 0; i < data->CmdListsCount; i++) {
        ImDrawList *cmdList = data->CmdLists[i];

        // TODO: static draw?
        vertices->setData(&cmdList->VtxBuffer.front(), cmdList->VtxBuffer.size() * sizeof(ImDrawVert), GL_STATIC_DRAW);
        indices->setData(&cmdList->IdxBuffer.front(), cmdList->IdxBuffer.size() * sizeof(ImDrawIdx), GL_STATIC_DRAW);

        int offset = 0;

        for (int cmdId = 0; cmdId < cmdList->CmdBuffer.size(); cmdId++) {
            ImDrawCmd *cmd = &cmdList->CmdBuffer[cmdId];

            if (cmd->UserCallback) {
                cmd->UserCallback(cmdList, cmd);
            }
            else {
                PVTexture *fontTexture = (PVTexture *)cmd->TextureId;
                fontTexture->bind(0);

                glScissor((int)cmd->ClipRect.x, (int)(height - cmd->ClipRect.w), (int)(cmd->ClipRect.z - cmd->ClipRect.x), (int)(cmd->ClipRect.w - cmd->ClipRect.y));

                glDrawElements(GL_TRIANGLES, cmd->ElemCount, GL_UNSIGNED_SHORT, (void *)offset);
                offset += cmd->ElemCount * sizeof(ImDrawIdx);

                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    glBindVertexArray(0);
    glUseProgram(0);
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
#if 0
    glfwWindowHint(GLFW_SRGB_CAPABLE, true);
#endif

    if (!(window = glfwCreateWindow(width, height, "Window", NULL, NULL))) {
        printf("Error opening glfw window\n");
        return;
    }

#if 0
    pixels = new float[image->getWidth() * image->getHeight() * 4]; // TODO delete
#endif

    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, glfw_mouse_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetCharCallback(window, glfw_char_callback);

    glfwSwapInterval(vsync);

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

    shader = new PVShader("ui_vertex.glsl", "ui_fragment.glsl");

    viewSizeLocation = shader->getUniformLocation("viewSize");

    ImGuiIO & io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.RenderDrawListsFn = imgui_render;
#if 0
    io.SetClipboardTextFn = set_clipboard;
    io.GetClipboardTextFn = get_clipboard;
#endif
    io.ClipboardUserData = window;

    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    ImFontConfig config;
    config.OversampleH = 4;
    config.OversampleV = 4;
    io.Fonts->AddFontFromFileTTF(relToExeDir("content/fonts/OpenSans-Semibold.ttf").c_str(), 18.0f, &config);

    ImGuiStyle & style = ImGui::GetStyle();

    style.WindowRounding = 0.0f;
    style.Alpha = 1.0f;
    style.Colors[ImGuiCol_TitleBg]          = ImVec4(0.1f, 0.2f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.2f, 0.4f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(0.16f, 0.26f, 0.46f, 1.0f);

    unsigned char *font_pixels;
    int font_width, font_height;
    io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);

    PVTexture *texture = new PVTexture(PVTextureType2D, PVPixelFormatRGBA8Unorm, font_width, font_height, 0, 1);
    texture->setPixels(0, 0, 0, 0, font_width, font_height, font_pixels);

    // TODO: mipmapping may help the UI look better/run faster
    uiSampler = new PVSampler(PVFilterNearest, PVFilterNearest, PVFilterNone, PVAddressClamp, PVAddressClamp, 1);

    io.Fonts->TexID = texture; // TODO: delete

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    vertices = new PVBuffer();
    vertices->bind(GL_ARRAY_BUFFER);

    indices = new PVBuffer();
    indices->bind(GL_ELEMENT_ARRAY_BUFFER);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 20, (void *)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 20, (void *)8);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, true, 20, (void *)16);

    glBindVertexArray(0);

    glfwShowWindow(window);

    glfwGetCursorPos(window, &mouseX, &mouseY);

    for (int i = 0; i < MAX_FRAMES; i++)
        cpuFrameTimes[i] = 0.0f;

    for (int i = 0; i < MAX_FRAMES; i++)
        gpuFrameTimes[i] = 0.0f;

    culling = true;
    maxLights = 4;
    scroll = 0.0f;
    mouseDown = false;

    glGenQueries(MAX_QUERIES, queries); // TODO: delete
    GLCHECK();

    lightIcon = PVTexture::fromFile("content/textures/light_icon.png", true);

    glGetIntegerv(GL_MAX_SAMPLES, &maxSampleCount);

    colorTexture = new PVTexture(PVTextureType2D, PVPixelFormatRGBA8Unorm, 64, 64, 0, 1);
    depthTexture = new PVTexture(PVTextureType2D, PVPixelFormatDepth32Float, 64, 64, 0, 1);

    framebuffer = new PVFramebuffer();
    framebuffer->setColorAttachment(0, colorTexture);
    framebuffer->setDepthAttachment(depthTexture);

    shadowTexture = new PVTexture(PVTextureType2D, PVPixelFormatDepth32Float, 64, 64, 0, 1);

    shadowFramebuffer = new PVFramebuffer();
    shadowFramebuffer->setDepthAttachment(shadowTexture);

    solidShader = new PVShader("solid_vertex.glsl", "solid_fragment.glsl");

    solidVB = new PVBuffer();

    solidVA = new PVVertexArray();

    solidVA->setVertexBuffer(solidVB);

    solidVA->setVertexAttribute(0, 3, PVVertexFormatFloat, false, 28, 0);
    solidVA->setVertexAttribute(1, 4, PVVertexFormatFloat, false, 28, 12); // TODO: uchar4

    currTime = glfwGetTime();
}

ImageDisplay::~ImageDisplay() {
    glfwDestroyWindow(window);

    glfwTerminate();
}

bool ImageDisplay::shouldClose() {
    return glfwWindowShouldClose(window);
}

/*
 * Copy the texture to video memory and redraw the image
 * on screen
 */
void ImageDisplay::refresh() {
#if 0
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
#endif
}

void ImageDisplay::swap() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}

float2 ImageDisplay::getCursorPos() {
    double cx, cy;
    glfwGetCursorPos(window, &cx, &cy);

    return float2(cx, cy);
}

struct SolidVertex {
    float position[3];
    float color[4];
};

struct SolidDrawCmd {
    GLenum fillMode;
    int vertexOffset;
    int vertexCount;
    bool clip;
    float4 clipPlane;
};

void drawWireBox(const AABB & bounds, const float4x4 & transform, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    SolidDrawCmd cmd;

    cmd.fillMode = GL_LINE;
    cmd.vertexOffset = vertices.size();
    cmd.clip = false;

    vertices.push_back({ { bounds.min.x, bounds.min.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.max.x, bounds.min.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.min.x, bounds.max.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.max.x, bounds.max.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.min.x, bounds.min.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.max.x, bounds.min.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.min.x, bounds.max.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.max.x, bounds.max.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.min.x, bounds.min.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.min.x, bounds.min.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.max.x, bounds.min.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.max.x, bounds.min.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.min.x, bounds.max.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.min.x, bounds.max.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.max.x, bounds.max.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.max.x, bounds.max.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.min.x, bounds.min.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.min.x, bounds.max.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.max.x, bounds.min.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.max.x, bounds.max.y, bounds.min.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.min.x, bounds.min.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.min.x, bounds.max.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { bounds.max.x, bounds.min.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { bounds.max.x, bounds.max.y, bounds.max.z }, { color.x, color.y, color.z, color.w } });

    cmd.vertexCount = vertices.size() - cmd.vertexOffset;
    commands.push_back(cmd);

    for (int i = cmd.vertexOffset; i < cmd.vertexOffset + cmd.vertexCount; i++) {
        float3 position = (transform * float4(vertices[i].position[0], vertices[i].position[1], vertices[i].position[2], 1.0f)).xyz();

        for (int j = 0; j < 3; j++)
            vertices[i].position[j] = position[j];
    }
}

void drawFilledCylinder(const float4x4 & transform, int numVertices, float r, float h, bool capTop, bool capBottom, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    SolidDrawCmd cmd;

    cmd.fillMode = GL_FILL;
    cmd.vertexOffset = vertices.size();
    cmd.clip = false;

    for (int i = 0; i < numVertices; i++) {
        // TODO: reuse first vertex
        float theta0 = (float)i / (float)numVertices;
        float theta1 = (float)(i + 1) / (float)numVertices;

        float x0 = r * cosf(theta0 * 2.0f * (float)M_PI);
        float z0 = r * sinf(theta0 * 2.0f * (float)M_PI);

        float x1 = r * cosf(theta1 * 2.0f * (float)M_PI);
        float z1 = r * sinf(theta1 * 2.0f * (float)M_PI);

        if (capBottom) {
            vertices.push_back({ { x0, 0, z0 }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ { x1, 0, z1 }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ {  0, 0, 0  }, { color.x, color.y, color.z, color.w } });
        }

        vertices.push_back({ { x0, 0, z0 }, { color.x, color.y, color.z, color.w } });
        vertices.push_back({ { x0, h, z0 }, { color.x, color.y, color.z, color.w } });
        vertices.push_back({ { x1, h, z1 }, { color.x, color.y, color.z, color.w } });

        vertices.push_back({ { x0, 0, z0 }, { color.x, color.y, color.z, color.w } });
        vertices.push_back({ { x1, h, z1 }, { color.x, color.y, color.z, color.w } });
        vertices.push_back({ { x1, 0, z1 }, { color.x, color.y, color.z, color.w } });

        if (capTop) {
            vertices.push_back({ { x0, h, z0 }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ { x1, h, z1 }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ {  0, h, 0  }, { color.x, color.y, color.z, color.w } });
        }
    }

    cmd.vertexCount = vertices.size() - cmd.vertexOffset;
    commands.push_back(cmd);

    for (int i = cmd.vertexOffset; i < cmd.vertexOffset + cmd.vertexCount; i++) {
        float3 position = (transform * float4(vertices[i].position[0], vertices[i].position[1], vertices[i].position[2], 1.0f)).xyz();

        for (int j = 0; j < 3; j++)
            vertices[i].position[j] = position[j];
    }
}

void drawFilledCone(const float4x4 & transform, int numVertices, float r, float h, bool capBottom, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    SolidDrawCmd cmd;

    cmd.fillMode = GL_FILL;
    cmd.vertexOffset = vertices.size();
    cmd.clip = false;

    for (int i = 0; i < numVertices; i++) {
        // TODO: reuse first vertex
        float theta0 = (float)i / (float)numVertices;
        float theta1 = (float)(i + 1) / (float)numVertices;

        float x0 = r * cosf(theta0 * 2.0f * (float)M_PI);
        float z0 = r * sinf(theta0 * 2.0f * (float)M_PI);

        float x1 = r * cosf(theta1 * 2.0f * (float)M_PI);
        float z1 = r * sinf(theta1 * 2.0f * (float)M_PI);

        if (capBottom) {
            vertices.push_back({ { x0, 0, z0 }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ { x1, 0, z1 }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ {  0, 0, 0  }, { color.x, color.y, color.z, color.w } });
        }

        vertices.push_back({ { x0, 0, z0 }, { color.x, color.y, color.z, color.w } });
        vertices.push_back({ { x1, 0, z1 }, { color.x, color.y, color.z, color.w } });
        vertices.push_back({ {  0, h, 0  }, { color.x, color.y, color.z, color.w } });
    }

    cmd.vertexCount = vertices.size() - cmd.vertexOffset;
    commands.push_back(cmd);

    for (int i = cmd.vertexOffset; i < cmd.vertexOffset + cmd.vertexCount; i++) {
        float3 position = (transform * float4(vertices[i].position[0], vertices[i].position[1], vertices[i].position[2], 1.0f)).xyz();

        for (int j = 0; j < 3; j++)
            vertices[i].position[j] = position[j];
    }
}

void drawFilledQuad(const float4x4 & transform, float w, float h, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    SolidDrawCmd cmd;

    cmd.fillMode = GL_FILL;
    cmd.vertexOffset = vertices.size();
    cmd.clip = false;

    vertices.push_back({ { 0, 0, 0 }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { 0, h, 0 }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { w, h, 0 }, { color.x, color.y, color.z, color.w } });

    vertices.push_back({ { 0, 0, 0 }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { w, h, 0 }, { color.x, color.y, color.z, color.w } });
    vertices.push_back({ { w, 0, 0 }, { color.x, color.y, color.z, color.w } });

    cmd.vertexCount = vertices.size() - cmd.vertexOffset;
    commands.push_back(cmd);

    for (int i = cmd.vertexOffset; i < cmd.vertexOffset + cmd.vertexCount; i++) {
        float3 position = (transform * float4(vertices[i].position[0], vertices[i].position[1], vertices[i].position[2], 1.0f)).xyz();

        for (int j = 0; j < 3; j++)
            vertices[i].position[j] = position[j];
    }
}

void drawFilledSphere(const float4x4 & transform, int numVerticesX, int numVerticesY, float r, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    SolidDrawCmd cmd;

    cmd.fillMode = GL_FILL;
    cmd.vertexOffset = vertices.size();
    cmd.clip = false;

    for (int i = 0; i < numVerticesY; i++) {
        // TODO: reuse first vertex
        float phi0 = (float)i / (float)numVerticesY * (float)M_PI;
        float phi1 = (float)(i + 1) / (float)numVerticesY * (float)M_PI;

        for (int j = 0; j < numVerticesX; j++) {
            float theta0 = (float)j / (float)numVerticesX * 2.0f * (float)M_PI;
            float theta1 = (float)(j + 1) / (float)numVerticesX * 2.0f * (float)M_PI;

            float3 p0(r * cosf(theta0) * sinf(phi0), r * cosf(phi0), r * sinf(theta0) * sinf(phi0));
            float3 p1(r * cosf(theta0) * sinf(phi1), r * cosf(phi1), r * sinf(theta0) * sinf(phi1));
            float3 p2(r * cosf(theta1) * sinf(phi1), r * cosf(phi1), r * sinf(theta1) * sinf(phi1));
            float3 p3(r * cosf(theta1) * sinf(phi0), r * cosf(phi0), r * sinf(theta1) * sinf(phi0));

            vertices.push_back({ { p0.x, p0.y, p0.z }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ { p1.x, p1.y, p1.z }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ { p2.x, p2.y, p2.z }, { color.x, color.y, color.z, color.w } });

            vertices.push_back({ { p0.x, p0.y, p0.z }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ { p2.x, p2.y, p2.z }, { color.x, color.y, color.z, color.w } });
            vertices.push_back({ { p3.x, p3.y, p3.z }, { color.x, color.y, color.z, color.w } });
        }
    }

    cmd.vertexCount = vertices.size() - cmd.vertexOffset;
    commands.push_back(cmd);

    for (int i = cmd.vertexOffset; i < cmd.vertexOffset + cmd.vertexCount; i++) {
        float3 position = (transform * float4(vertices[i].position[0], vertices[i].position[1], vertices[i].position[2], 1.0f)).xyz();

        for (int j = 0; j < 3; j++)
            vertices[i].position[j] = position[j];
    }
}

void drawWireCircle(const float4x4 & transform, int numVertices, float r, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    SolidDrawCmd cmd;

    cmd.fillMode = GL_LINE;
    cmd.vertexOffset = vertices.size();
    cmd.clip = false;

    for (int i = 0; i < numVertices; i++) {
        // TODO: reuse first vertex
        float theta0 = (float)i / (float)numVertices;
        float theta1 = (float)(i + 1) / (float)numVertices;

        float x0 = r * cosf(theta0 * 2.0f * (float)M_PI);
        float y0 = r * sinf(theta0 * 2.0f * (float)M_PI);

        float x1 = r * cosf(theta1 * 2.0f * (float)M_PI);
        float y1 = r * sinf(theta1 * 2.0f * (float)M_PI);

        vertices.push_back({ { x0, y0, 0 }, { color.x, color.y, color.z, color.w } });
        vertices.push_back({ { x1, y1, 0 }, { color.x, color.y, color.z, color.w } });
    }

    cmd.vertexCount = vertices.size() - cmd.vertexOffset;
    commands.push_back(cmd);

    for (int i = cmd.vertexOffset; i < cmd.vertexOffset + cmd.vertexCount; i++) {
        float3 position = (transform * float4(vertices[i].position[0], vertices[i].position[1], vertices[i].position[2], 1.0f)).xyz();

        for (int j = 0; j < 3; j++)
            vertices[i].position[j] = position[j];
    }
}

// TODO ::rotation() is broken

void drawAxes(const float4x4 & transform, int numVertices, float lineRadius, float lineLength, float coneRadius, float coneLength, const float4 & xcolor, const float4 & ycolor, const float4 & zcolor, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    float4x4 cylTransform = transform;
    drawFilledCylinder(cylTransform, numVertices, lineRadius, lineLength, false, true, ycolor, vertices, commands);
    float4x4 coneTransform = cylTransform * translation(float3(0.0f, lineLength, 0.0f));
    drawFilledCone(coneTransform, numVertices, coneRadius, coneLength, true, ycolor, vertices, commands);

    cylTransform = transform * rotationX((float)M_PI / 2.0f);
    drawFilledCylinder(cylTransform, numVertices, lineRadius, lineLength, false, true, zcolor, vertices, commands);
    coneTransform = cylTransform * translation(float3(0.0f, lineLength, 0.0f));
    drawFilledCone(coneTransform, numVertices, coneRadius, coneLength, true, zcolor, vertices, commands);

    cylTransform = transform * rotationZ(-(float)M_PI / 2.0f);
    drawFilledCylinder(cylTransform, numVertices, lineRadius, lineLength, false, true, xcolor, vertices, commands);
    coneTransform = cylTransform * translation(float3(0.0f, lineLength, 0.0f));
    drawFilledCone(coneTransform, numVertices, coneRadius, coneLength, true, xcolor, vertices, commands);
}

void flushSolidDrawCommands(const float4x4 & viewProjectionMatrix, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    if (vertices.size() == 0)
        return;

    GLCHECK(glEnable(GL_BLEND));
    GLCHECK(glBlendEquation(GL_FUNC_ADD));
    GLCHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    solidVB->setData(&vertices[0], sizeof(SolidVertex) * vertices.size(), GL_STATIC_DRAW);

    solidVA->bind();
    solidShader->bind();

    GLint viewProjectionLocation = solidShader->getUniformLocation("viewProjection");
    GLCHECK(glUniformMatrix4fv(viewProjectionLocation, 1, true, &viewProjectionMatrix.rows[0][0]));

    for (SolidDrawCmd & command : commands) {
        // TODO: sort/coalesce commands. Could probably do so for imgui as well.
        GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, command.fillMode));
        GLCHECK(glDrawArrays(command.fillMode == GL_LINE ? GL_LINES : GL_TRIANGLES, command.vertexOffset, command.vertexCount));
    }

    GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    GLCHECK(glUseProgram(0));
    GLCHECK(glBindVertexArray(0));
    GLCHECK(glDisable(GL_BLEND));
}

void transformGizmo(bool mouseDown, float4x4 & transform, const float4x4 & viewProjection, const float2 & viewPos, const float2 & viewSize, float pixelSize, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands) {
    float4x4 tp = transpose(transform);

    float3 c0 = tp.rows[0].xyz();
    float3 c1 = tp.rows[1].xyz();
    float3 c2 = tp.rows[2].xyz();

    float3 origin = tp.rows[3].xyz();

    float3 scale(length(c0), length(c1), length(c2));

    c0 = c0 / scale.x;
    c1 = c1 / scale.y;
    c2 = c2 / scale.z;

    float3x3 rotation;
    rotation.rows[0] = c0;
    rotation.rows[1] = c1;
    rotation.rows[2] = c2;
    rotation = transpose(rotation);

    float axisLength = 0.0f;

    float4x4 invViewProjection = inverse(viewProjection);

    // Compute gizmo scale
    {
        // Transform gizmo position into screen space
        float4 positionNDC(origin, 1);
        positionNDC = viewProjection * positionNDC;
        positionNDC = positionNDC / positionNDC.w;

        // Create another point a constant screen space distance away
        float desiredSize = 100.0f / viewSize.x * 2.0f;
        float4 offsetPosNDC = positionNDC;
        offsetPosNDC.x += desiredSize;

        // Unproject second point
        float4 offsetPos = invViewProjection * offsetPosNDC;
        offsetPos = offsetPos / offsetPos.w;

        // Get world space distance between two points
        axisLength = length(offsetPos.xyz() - origin);
    }

    float axisRadius = 0.022f * axisLength;
    float quadLength = axisLength * 0.33f;

    Ray ray;

    // Compute view ray
    {
        float2 ndc(mouseX, mouseY);
        ndc = (ndc - viewPos) / viewSize;
        ndc = ndc * 2.0f - 1.0f;
        ndc.y = -ndc.y;

        float4 near(ndc, 0.0f, 1.0f); // TODO: -1 or 0 ?
        float4 far(ndc, 1.0f, 1.0f);

        near = invViewProjection * near;
        far = invViewProjection * far;
        near = near / near.w;
        far = far / far.w;

        ray = Ray(near.xyz(), normalize(far.xyz() - near.xyz()));
    }

    float3 axes[3] = {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1)
    };

    // Don't allow world space scaling because it leads to skewing
    if (local || transformMode == TransformModeScale)
        for (int i = 0; i < 3; i++)
            axes[i] = rotation * axes[i];

    Plane planes[3] = {
        Plane(axes[0], dot(origin, axes[0])),
        Plane(axes[1], dot(origin, axes[1])),
        Plane(axes[2], dot(origin, axes[2]))
    };

    float3 axisColors[3];
    float3 quadColors[3];

    for (int i = 0; i < 3; i++) {
        axisColors[i] = float3(i == 0 ? 1.0f : 0.0f, i == 1 ? 1.0f : 0.0f, i == 2 ? 1.0f : 0.0f);
        quadColors[i] = float3(i == 0 ? 1.0f : 0.0f, i == 1 ? 1.0f : 0.0f, i == 2 ? 1.0f : 0.0f);
    }

    if (draggingTransform) {
        float hitDist;
        if (planes[selectedPlane].intersects(ray, hitDist)) {
            float3 hitPos = ray.at(hitDist) - origin;

            float3 delta = hitPos - selectedOffset;
            float3 constrainedDelta = 0;

            // TODO: can just filter out the unselected axes directly
            for (int i = 0; i < 3; i++) {
                if ((1 << i) & selectedAxes)
                    constrainedDelta = constrainedDelta + dot(axes[i], delta) * axes[i];
            }

            if (transformMode == TransformModeTranslation) {
                origin = constrainedDelta + origin;
                // Does not affect offset
            }
            else if (transformMode == TransformModeScale) {
                float3 scaleAmount(1, 1, 1);

#if 1
                // TODO: clamp second dot product
                for (int i = 0; i < 3; i++) {
                    if ((1 << i) & selectedAxes)
                        scaleAmount[i] = max(dot(axes[i], hitPos), 0.00001f) / max(dot(axes[i], selectedOffset), 0.00001f);
                }
#endif

                scale = scaleAmount * scale;

                // Scales offset
                selectedOffset = scaleAmount[0] * dot(selectedOffset, axes[0]) * axes[0] +
                                 scaleAmount[1] * dot(selectedOffset, axes[1]) * axes[1] +
                                 scaleAmount[2] * dot(selectedOffset, axes[2]) * axes[2];
            }
            else if (transformMode == TransformModeRotation) {
                float3 S_norm = normalize(selectedOffset);

                float3 HonS = dot(hitPos, S_norm) * S_norm;
                float3 HoffS = hitPos - HonS;

                float theta = atan2(length(HoffS), length(HonS));

                if (dot(cross(S_norm, hitPos), planes[selectedPlane].normal) < 0.0f)
                    theta = -theta;

                float3x3 rotationMatrix = upper3x3(::rotation(planes[selectedPlane].normal, theta));

                rotation = rotationMatrix * rotation;

                // Rotates offset
                selectedOffset = rotationMatrix * selectedOffset;
            }

            float4x4 fullRotation;
            fullRotation.rows[0] = float4(rotation.rows[0], 0);
            fullRotation.rows[1] = float4(rotation.rows[1], 0);
            fullRotation.rows[2] = float4(rotation.rows[2], 0);

            transform = translation(origin) * fullRotation * ::scale(scale);
        }

        for (int i = 0; i < 3; i++) {
            if ((1 << i) & selectedAxes)
                axisColors[i] = float3(1, 1, 0);

            if (((1 << ((i + 1) % 3)) & selectedAxes) && ((1 << ((i + 2) % 3)) & selectedAxes))
                quadColors[i] = float3(1, 1, 0);
        }
    }
    else if (!draggingCamera) {
        int hoveredAxes = 0;
        int hoveredPlane;
        float hoveredDist = INFINITY;

        if (transformMode == TransformModeTranslation || transformMode == TransformModeScale) {
            for (int pickAxis = 0; pickAxis < 3; pickAxis++) {
                // TODO: pick the plane with the greatest precision?
                for (int pickPlane = 0; pickPlane < 3; pickPlane++) {
                    if (pickPlane == pickAxis)
                        continue;

                    float hitDist;
                    if (planes[pickPlane].intersects(ray, hitDist)) {
                        float3 hitPos = ray.at(hitDist) - origin;
                        float onAxisAmt = dot(axes[pickAxis], hitPos);
                        float3 onAxis = onAxisAmt * axes[pickAxis];
                        float3 offAxis = hitPos - onAxis;
                        float offAxisAmt = length(offAxis); // TODO: probably a simpler expression

                        // TODO: Could take cone radius into account
                        if (onAxisAmt > 0.0f && onAxisAmt < axisLength && offAxisAmt < axisRadius * 4.0f && hitDist < hoveredDist) {
                            hoveredAxes = (1 << pickAxis);
                            hoveredDist = hitDist;
                            hoveredPlane = pickPlane;
                        }
                    }
                }
            }

            for (int pickPlane = 0; pickPlane < 3; pickPlane++) {
                int axis0 = (pickPlane + 1) % 3;
                int axis1 = (pickPlane + 2) % 3;

                float hitDist;
                if (planes[pickPlane].intersects(ray, hitDist)) {
                    float3 hitPos = ray.at(hitDist) - origin;

                    float onAxis0 = dot(axes[axis0], hitPos);
                    float onAxis1= dot(axes[axis1], hitPos);

                    // TODO: Selection radius should probably be in screen space

                    if (onAxis0 > axisRadius * 4.0f && onAxis0 < quadLength && onAxis1 > axisRadius * 4.0f && onAxis1 < quadLength && hitDist < hoveredDist) {
                        hoveredAxes = (1 << axis0) | (1 << axis1);
                        hoveredDist = hitDist;
                        hoveredPlane = pickPlane;
                    }
                }
            }
        }
        else if (transformMode == TransformModeRotation) {
            for (int pickPlane = 0; pickPlane < 3; pickPlane++) {
                float hitDist;
                if (planes[pickPlane].intersects(ray, hitDist)) {
                    float3 hitPos = ray.at(hitDist) - origin;

                    float r = length(hitPos);

                    if (r > axisLength * 0.8f && r < axisLength * 1.2f) {
                        hoveredAxes = (1 << ((pickPlane + 1) % 3)) | (1 << ((pickPlane + 2) %3));
                        hoveredDist = hitDist;
                        hoveredPlane = pickPlane;
                    }
                }
            }
        }

        if (hoveredAxes != 0) {
            if (mouseDown) {
                selectedAxes = hoveredAxes;
                selectedOffset = ray.at(hoveredDist) - origin;
                selectedPlane = hoveredPlane;
                draggingTransform = true;
            }

            for (int i = 0; i < 3; i++) {
                if ((1 << i) & hoveredAxes)
                    axisColors[i] = float3(1, 1, 0);

                if (((1 << ((i + 1) % 3)) & hoveredAxes) && ((1 << ((i + 2) % 3)) & hoveredAxes))
                    quadColors[i] = float3(1, 1, 0);
            }

        }
    }

    float4x4 gizmoTransform = translation(origin);

    if (local || transformMode == TransformModeScale) {
        float4x4 fullRotation;
        fullRotation.rows[0] = float4(rotation.rows[0], 0);
        fullRotation.rows[1] = float4(rotation.rows[1], 0);
        fullRotation.rows[2] = float4(rotation.rows[2], 0);

        gizmoTransform = gizmoTransform * fullRotation;
    }

    if (transformMode == TransformModeTranslation || transformMode == TransformModeScale) {
        drawAxes(gizmoTransform, 8, axisRadius, 0.66f * axisLength, 0.11f * axisLength, 0.33f * axisLength, float4(axisColors[0], 1), float4(axisColors[1], 1), float4(axisColors[2], 1), vertices, commands);

        // TODO: Sort for blending
        float4x4 quadTransform;
        drawFilledQuad(gizmoTransform * quadTransform, quadLength, quadLength, float4(quadColors[2], 0.5f), vertices, commands);
        quadTransform = rotationX((float)M_PI / 2.0f);
        drawFilledQuad(gizmoTransform * quadTransform, quadLength, quadLength, float4(quadColors[1], 0.5f), vertices, commands);
        quadTransform = rotationY(-(float)M_PI / 2.0f);
        drawFilledQuad(gizmoTransform * quadTransform, quadLength, quadLength, float4(quadColors[0], 0.5f), vertices, commands);
    }
    else if (transformMode == TransformModeRotation) {
        float4x4 circleTransform;
        drawWireCircle(gizmoTransform * circleTransform, 32, axisLength, float4(quadColors[2], 1.0f), vertices, commands);
        circleTransform = rotationX((float)M_PI / 2.0f);
        drawWireCircle(gizmoTransform * circleTransform, 32, axisLength, float4(quadColors[1], 1.0f), vertices, commands);
        circleTransform = rotationY(-(float)M_PI / 2.0f);
        drawWireCircle(gizmoTransform * circleTransform, 32, axisLength, float4(quadColors[0], 1.0f), vertices, commands);

        for (int i = 0; i < 3; i++) {
            SolidDrawCmd & cmd = commands[commands.size() - 1 - i];

            cmd.clip = true;
            //cmd.clipPlane = float4()
        }
    }
}

void ImageDisplay::drawPreviewScene(PVScene *scene) {
    ImGuiIO & io = ImGui::GetIO();

    double newTime = glfwGetTime();
    double elapsed = newTime - currTime;
    currTime = newTime;

    double newMouseX, newMouseY;
    glfwGetCursorPos(window, &newMouseX, &newMouseY);

    bool prevMouseDown = mouseDown;
    mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    double deltaMouseX = newMouseX - mouseX;
    double deltaMouseY = newMouseY - mouseY;
    mouseX = newMouseX;
    mouseY = newMouseY;

    int width, height, windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &width, &height);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    io.DisplaySize.x = windowWidth;
    io.DisplaySize.y = windowHeight;
    io.DisplayFramebufferScale.x = (float)width / (float)windowWidth;
    io.DisplayFramebufferScale.y = (float)height / (float)windowHeight;
    io.DeltaTime = elapsed;
    io.MousePos = ImVec2(newMouseX, newMouseY);
    io.MouseWheel = scroll * 0.5f;

    float deltaScroll = scroll;
    scroll = 0.0f;

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

#if 1
    {
        ImGui::SetNextWindowPos(ImVec2(350, windowHeight - 300), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Renderer Settings", nullptr, ImVec2(440, 300));

        for (int i = 0; i < MAX_FRAMES - 1; i++)
            cpuFrameTimes[i] = cpuFrameTimes[i + 1];
        cpuFrameTimes[MAX_FRAMES - 1] = /*timer.getElapsedMilliseconds();*/ elapsed * 1000.0f;

        float avgCPU = 0.0f;

        for (int i = 0; i < MAX_FRAMES; i++)
            avgCPU += cpuFrameTimes[i];

        avgCPU /= MAX_FRAMES;

        float avgGPU = 0.0f;

        for (int i = 0; i < MAX_FRAMES; i++)
            avgGPU += gpuFrameTimes[i];

        avgGPU /= MAX_FRAMES;

        char avgCPUStr[256];
        memset(avgCPUStr, 0, 256);
        sprintf(avgCPUStr, "Average: %.02f", avgCPU);

        char avgGPUStr[256];
        memset(avgGPUStr, 0, 256);
        sprintf(avgGPUStr, "Average: %.02f", avgGPU);

        ImGui::Text("Resolution: %dx%d", width / 2, height / 2);
        ImGui::PlotLines("CPU Frame Time", cpuFrameTimes, MAX_FRAMES, 0, avgCPUStr, 0.0f, 33.0f, ImVec2(0,50), sizeof(float));
        ImGui::PlotLines("GPU Frame Time", gpuFrameTimes, MAX_FRAMES, 0, avgGPUStr, 0.0f, 33.0f, ImVec2(0,50), sizeof(float));

        ImGui::Checkbox("Backface Culling", &culling);

        if (ImGui::Checkbox("VSync", &vsync))
            glfwSwapInterval(vsync);

        ImGui::SliderInt("Max Lights", &maxLights, 1, 32);

        const char *filterModes[] = {
            "Nearest",
            "Bilinear",
            "Trilinear",
            "Anisotropic"
        };

        ImGui::Combo("Filter Mode", (int *)&filterMode, filterModes, sizeof(filterModes) / sizeof(filterModes[0]));

        ImGui::SliderInt("Anisotropic Samples", &anisotropy, 1, 16);

        ImGui::Checkbox("Enable Lighting", &lighting);
        ImGui::Checkbox("Enable Textures", &texturing);
        ImGui::Checkbox("Enable Normal Maps", &normalMapping);
        ImGui::Checkbox("Wireframe", &wireframe);

        ImGui::SliderInt("MSAA Samples", &sampleCount, 0, maxSampleCount);

        ImGui::End();
    }
#endif
#if 1
    {
        // TODO: This window doesn't need a background, especially a transparent one
        ImGui::SetNextWindowPos(ImVec2(350, 0));
        ImGui::SetNextWindowSize(ImVec2(windowWidth - 700, windowHeight - 300));

        ImGui::Begin("View", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 padding = ImGui::GetStyle().WindowPadding;
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos() - padding;
        ImVec2 sz = ImGui::GetContentRegionAvail() + padding * 2;

        ImRect bb(pos, pos + sz);

        ImGui::ItemSize(bb);
        const ImGuiID id = ImGui::GetCurrentWindow()->GetID("view_button");

        if (ImGui::ItemAdd(bb, &id)) {
            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

            int viewWidth = sz.x;
            int viewHeight = sz.y;

            float3 camTarg = scene->getCamera(0)->getTarget();
            float3 camPos = scene->getCamera(0)->getPosition() - camTarg;

            float r = length(camPos);
            float theta = atan2(camPos.z, camPos.x);
            float phi = acos(camPos.y / r);

            // TODO: pressed and held trigger together

            if (held && !draggingTransform) {
                if (deltaMouseX != 0 || deltaMouseY != 0)
                    draggingCamera = true;

                if (draggingCamera) {
                    if (!io.KeySuper) {
                        theta += deltaMouseX * -0.004f;
                        phi += deltaMouseY * -0.004f;

                        if (phi < 0.001f)
                            phi = 0.001f;

                        if (phi > (float)M_PI * 0.9999f)
                            phi = (float)M_PI * 0.9999f;
                    }
                    else {
                        // TODO: we could project the actual mouse delta in world space
                        camTarg = camTarg + deltaMouseX * -0.03f * scene->getCamera(0)->getRight()
                                    + deltaMouseY *  0.03f * scene->getCamera(0)->getUp();
                    }
                }
            }

            if (hovered) {
                r += deltaScroll * -0.1f;

                if (r < 0.001f)
                    r = 0.001f;
            }

            camPos.x = r * cos(theta) * sin(phi);
            camPos.y = r * cos(phi);
            camPos.z = r * sin(theta) * sin(phi);

            scene->getCamera(0)->setTarget(camTarg);
            scene->getCamera(0)->setPosition(camPos + camTarg);

            Timer timer;

#define ENABLE_GPU_PROFILING 1

#if ENABLE_GPU_PROFILING
            glBeginQuery(GL_TIME_ELAPSED, queries[frameIdx % 3]);
#endif

            if (colorTexture->getWidth() != viewWidth || colorTexture->getHeight() != viewHeight || colorTexture->getSampleCount() != sampleCount) {
                delete colorTexture;
                delete depthTexture;
                delete framebuffer;

                if (resolveTexture) {
                    delete resolveTexture;
                    delete resolveFramebuffer;

                    resolveTexture = nullptr;
                    resolveFramebuffer = nullptr;
                }

                if (sampleCount > 0) {
                    colorTexture = new PVTexture(PVTextureType2DMultisample, PVPixelFormatRGBA8Unorm, viewWidth, viewHeight, sampleCount, 1);
                    depthTexture = new PVTexture(PVTextureType2DMultisample, PVPixelFormatDepth32Float, viewWidth, viewHeight, sampleCount, 1);

                    resolveTexture = new PVTexture(PVTextureType2D, PVPixelFormatRGBA8Unorm, viewWidth, viewHeight, 0, 1);

                    resolveFramebuffer = new PVFramebuffer();
                    resolveFramebuffer->setColorAttachment(0, resolveTexture);
                }
                else {
                    colorTexture = new PVTexture(PVTextureType2D, PVPixelFormatRGBA8Unorm, viewWidth, viewHeight, 0, 1);
                    depthTexture = new PVTexture(PVTextureType2D, PVPixelFormatDepth32Float, viewWidth, viewHeight, 0, 1);
                }

                framebuffer = new PVFramebuffer();
                framebuffer->setColorAttachment(0, colorTexture);
                framebuffer->setDepthAttachment(depthTexture);
            }

            float4x4 viewMatrix = lookAtLH(scene->getCamera(0)->getPosition(), scene->getCamera(0)->getTarget(), scene->getCamera(0)->getUp());
            float4x4 projectionMatrix = perspectiveLH(scene->getCamera(0)->getFOV(), (float)viewWidth / (float)viewHeight, 0.1f, 100.0f);
            float4x4 viewProjectionMatrix = projectionMatrix * viewMatrix;
            float3 viewPosition = scene->getCamera(0)->getPosition();
            float4x4 invViewProjection = inverse(viewProjectionMatrix);

            PVRenderFlags flags = (PVRenderFlags)(
                (texturing ? PVRenderFlagEnableTexturing : 0) |
                (normalMapping ? PVRenderFlagEnableNormalMapping : 0) |
                (culling ? PVRenderFlagEnableCulling : 0) |
                (lighting ? PVRenderFlagEnableLighting : 0) |
                (wireframe ? PVRenderFlagWireframe : 0)
            );

            framebuffer->bind(GL_DRAW_FRAMEBUFFER);

            if (sampleCount > 0)
                GLCHECK(glEnable(GL_MULTISAMPLE));

            GLCHECK(glViewport(0, 0, viewWidth, viewHeight));

            scene->draw(flags, maxLights, filterMode, anisotropy, viewMatrix, projectionMatrix, viewPosition);

            GLCHECK(glClearDepth(1.0f));
            GLCHECK(glClear(GL_DEPTH_BUFFER_BIT));

            GLCHECK(glEnable(GL_DEPTH_TEST));

            std::vector<SolidVertex> verts;
            std::vector<SolidDrawCmd> commands;

            // TODO: index buffer

            // TODO: Don't start dragging if super key is down

            if (selectedInstance) {
                const AABB & bounds = selectedInstance->mesh->getBounds();
                float4x4 transform = selectedInstance->getTransform();

                drawWireBox(bounds, transform, float4(1, 1, 1, 1), verts, commands);

                transformGizmo(held, transform, viewProjectionMatrix, float2(pos.x, pos.y), float2(sz.x, sz.y), 100.0f, verts, commands);

                decompose(transform, selectedInstance->position, selectedInstance->rotation, selectedInstance->scale);
            }

            flushSolidDrawCommands(viewProjectionMatrix, verts, commands);

            {
                float3x3 axisViewMatrix3 = upper3x3(transpose(inverse(viewMatrix)));
                float4x4 axisViewMatrix;

                for (int i = 0; i < 3; i++)
                    for (int j = 0; j < 3; j++)
                        axisViewMatrix.rows[i][j] = axisViewMatrix3.rows[i][j];

                axisViewMatrix = translation(float3(0, 0, 1)) * axisViewMatrix;
                float4x4 axisProjectionMatrix = orthographicLH(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
                float4x4 axisViewProjectionMatrix = axisProjectionMatrix * axisViewMatrix;

                // TODO: scale with view size
                GLCHECK(glViewport(viewWidth - 100, 0, 100, 100));
                GLCHECK(glClear(GL_DEPTH_BUFFER_BIT));

                verts.clear();
                commands.clear();

                drawAxes(float4x4(), 8, 0.02f, 0.66f, 0.11f, 0.33f, float4(1, 0, 0, 1), float4(0, 1, 0, 1), float4(0, 0, 1, 1), verts, commands);

                flushSolidDrawCommands(axisViewProjectionMatrix, verts, commands);
            }

            GLCHECK(glDisable(GL_DEPTH_TEST));

            if (sampleCount > 0)
                GLCHECK(glDisable(GL_MULTISAMPLE));

            GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

            if (sampleCount > 0)
                framebuffer->resolve(resolveFramebuffer);

#if ENABLE_GPU_PROFILING
            glEndQuery(GL_TIME_ELAPSED);
#endif

            float gpuFrameTime = 0.0f;

#if ENABLE_GPU_PROFILING
            if (++frameIdx >= 3) {
                int timerDone = 0;
                while (!timerDone)
                    glGetQueryObjectiv(queries[frameIdx % 3], GL_QUERY_RESULT_AVAILABLE, &timerDone);

                GLuint64 elapsedTime;
                glGetQueryObjectui64v(queries[frameIdx % 3], GL_QUERY_RESULT, &elapsedTime);

                gpuFrameTime = elapsedTime / 1000000.0f;
            }
#endif

            for (int i = 0; i < MAX_FRAMES - 1; i++)
                gpuFrameTimes[i] = gpuFrameTimes[i + 1];
            gpuFrameTimes[MAX_FRAMES - 1] = gpuFrameTime;

            PVTexture *viewTex = colorTexture;

            if (sampleCount > 0)
                viewTex = resolveTexture;

            ImGui::PushClipRect(pos, pos + sz, false);

            drawList->AddImage(viewTex, pos, pos + sz, ImVec2(0, 1), ImVec2(1, 0), IM_COL32(255, 255, 255, 255));

            if (pressed && !draggingCamera && !draggingTransform) {
                float2 ndc(mouseX, mouseY);
                ndc = (ndc - float2(pos.x, pos.y)) / float2(sz.x, sz.y);
                ndc = ndc * 2.0f - 1.0f;
                ndc.y = -ndc.y;

                float4x4 invViewProjection = inverse(viewProjectionMatrix);

                float4 near(ndc, 0.0f, 1.0f); // TODO: -1 or 0 ?
                float4 far(ndc, 1.0f, 1.0f);

                near = invViewProjection * near;
                far = invViewProjection * far;
                near = near / near.w;
                far = far / far.w;

                PVMeshInstance *closestHit = nullptr;
                float closestDist = INFINITY;

                for (int i = 0; i < scene->getNumMeshInstances(); i++) {
                    PVMeshInstance *instance = scene->getMeshInstance(i);

                    float4x4 instanceTransform = instance->getTransform();
                    float4x4 invInstanceTransform = inverse(instanceTransform);

                    float4 tfNear = invInstanceTransform * near;
                    float4 tfFar = invInstanceTransform * far;

                    // TODO: not necessary for affine transforms
                    tfNear = tfNear / tfNear.w;
                    tfFar = tfFar / tfFar.w;

                    float3 dir = normalize(tfFar.xyz() - tfNear.xyz());

                    Ray mouseRay(tfNear.xyz(), normalize(tfFar.xyz() - tfNear.xyz()));

                    Collision result;
                    if (instance->mesh->intersect(mouseRay, result)) {
                        float3 hitPos = mouseRay.at(result.distance);

                        // TODO: divide not needed for affine transforms
                        float4 tfHitPos = instanceTransform * float4(hitPos, 1.0f);
                        tfHitPos = tfHitPos / tfHitPos.w;

                        float dist = length(near.xyz() - tfHitPos.xyz());

                        if (dist < closestDist) {
                            closestHit = instance;
                            closestDist = dist;
                        }
                    }
                }

                if (closestHit) {
                    printf("Hit mesh instance '%s': %f\n", closestHit->name.c_str(), closestDist);
                
                    selectedInstance = closestHit;
                    selectedLight = nullptr;
                    selectedCamera = nullptr;
                }
            }

            for (int i = 0; i < scene->getNumLights(); i++) {
                PVLight *light = scene->getLight(i);
                float3 position = light->getPosition();

                float4 transformed = viewProjectionMatrix * float4(position, 1.0f);

                float depth = transformed.z / transformed.w;

                if (depth < 0.001f)
                    continue;

                float2 screenPos = transformed.xy() / transformed.w;

                screenPos = screenPos * 0.5f + 0.5f;
                screenPos.y = 1.0f - screenPos.y;
                screenPos = screenPos * float2(sz.x, sz.y) + float2(pos.x, pos.y);

                ImVec2 center = ImVec2(screenPos.x, screenPos.y);
                ImVec2 rad = ImVec2(20, 20);

                ImVec4 rgb(light->color.x, light->color.y, light->color.z, 1.0f);
                ImVec4 hsv(0.0f, 0.0f, 0.0f, rgb.w);
                ImGui::ColorConvertRGBtoHSV(rgb.x, rgb.y, rgb.z, hsv.x, hsv.y, hsv.z);
                hsv.z = 1.0f; // Don't allow icon to get too dark
                hsv.w = selectedLight == light ? 1.0f : 0.5f;
                ImGui::ColorConvertHSVtoRGB(hsv.x, hsv.y, hsv.z, rgb.x, rgb.y, rgb.z);
                rgb.w = hsv.w;

                drawList->AddImage(lightIcon, center - rad, center + rad, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(rgb));

                if (pressed && !draggingCamera && !draggingTransform && mouseX >= screenPos.x - rad.x && mouseX <= screenPos.x + rad.x && mouseY >= screenPos.y - rad.y && mouseY <= screenPos.y + rad.y) {
                    selectedInstance = nullptr;
                    selectedLight = light;
                    selectedCamera = nullptr;
                }
            }

            if (!held) {
                draggingCamera = false;
                draggingTransform = false;
                selectedAxes = 0;
            }

            ImGui::PopClipRect();
        }

        ImGui::End();
    }
#endif
#if 1
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(350, windowHeight));

        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (ImGui::TreeNode("Mesh Instances")) {
            for (int i = 0; i < scene->getNumMeshInstances(); i++) {
                PVMeshInstance *instance = scene->getMeshInstance(i);
                std::string name = instance->name;

                if (name == "")
                    name = "Unnamed Mesh Instance";

                ImGui::TreeNodeEx((void*)(intptr_t)i, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, "");
                ImGui::SameLine();
                ImGui::Checkbox(name.c_str(), &instance->visible);

                if (ImGui::IsItemClicked()) {
                    selectedInstance = instance;
                    selectedLight = nullptr;
                    selectedCamera = nullptr;
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Lights")) {
            for (int i = 0; i < scene->getNumLights(); i++) {
                ImGui::TreeNodeEx((void*)(intptr_t)i, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, "Light %d", i);

                if (ImGui::IsItemClicked()) {
                    selectedInstance = nullptr;
                    selectedLight = scene->getLight(i);
                    selectedCamera = nullptr;
                }
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Cameras")) {
            for (int i = 0; i < scene->getNumCameras(); i++) {
                ImGui::TreeNodeEx((void*)(intptr_t)i, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, "Camera %d", i);
                    
                if (ImGui::IsItemClicked()) {
                    selectedInstance = nullptr;
                    selectedLight = nullptr;
                    selectedCamera = scene->getCamera(i);

                    camFOV = selectedCamera->getFOV() * 180.0f / (float)M_PI;
                }
            }

            ImGui::TreePop();
        }

        ImGui::End();
    }
#endif
#if 1
    {
        ImGui::SetNextWindowPos(ImVec2(windowWidth - 350, 0));
        ImGui::SetNextWindowSize(ImVec2(350, windowHeight));

        ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (selectedInstance) {
            ImGui::ColorPicker("Diffuse Color", &selectedInstance->diffuseColor[0]);
            ImGui::ColorPicker("Specular Color", &selectedInstance->specularColor[0]);
            ImGui::DragFloat("Specular Power", &selectedInstance->specularPower, 0.5f, 1.0f, 64.0f);
            ImGui::DragFloat("Normal Map Scale", &selectedInstance->normalMapScale, 0.005f, 0.0f, 4.0f);

            ImGui::Text("Diffuse Texture");

            if (selectedInstance->diffuseTexture)
                ImGui::Image(selectedInstance->diffuseTexture, ImVec2(250, 250));
            else
                ImGui::Text("None");

            ImGui::Text("Normal Texture");

            if (selectedInstance->normalTexture)
                ImGui::Image(selectedInstance->normalTexture, ImVec2(250, 250));
            else
                ImGui::Text("None");

            #if 1
            ImGui::InputFloat3("Position", &selectedInstance->position[0], 3);
            ImGui::InputFloat3("Rotation", &selectedInstance->rotation[0], 3);
            ImGui::InputFloat3("Scale", &selectedInstance->scale[0], 3);
            #endif
        }
        else if (selectedLight) {
            ImGui::ColorPicker("Color", &selectedLight->color[0]);

            ImGui::DragFloat("Intensity", &selectedLight->intensity, 1.0f, 0.0f, 1000.0f);

            #if 0
            ImGui::InputFloat3("Position", &selectedLight->position[0], 3);
            #endif
        }
        else if (selectedCamera) {
            ImGui::DragFloat("FOV", &camFOV, 0.5f, 5.0f, 150.0f);
            selectedCamera->setFOV(camFOV / 180.0f * (float)M_PI);
        }

        ImGui::End();
    }
#endif

    // TODO: add a way to select nothing

    ImGui::Render();
}
