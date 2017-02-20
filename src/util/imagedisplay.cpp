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
#include <preview/solidgeom.h>
#include <preview/transformgizmo.h>

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

TransformGizmoState transformState;

uint64_t frameIdx = 0;

#define MAX_FRAMES 120
float cpuFrameTimes[MAX_FRAMES];
float gpuFrameTimes[MAX_FRAMES];

#define MAX_QUERIES 3
GLuint queries[MAX_QUERIES];

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
            transformState.mode = TransformModeTranslation;
            break;
        case 'e':
            transformState.mode = TransformModeRotation;
            break;
        case 'r':
            transformState.mode = TransformModeScale;
            break;
        case 'l':
            transformState.local = !transformState.local;
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

            if (held && !transformState.active) {
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

                transformGizmo(
                    transform,
                    viewProjectionMatrix,
                    held && !draggingCamera,
                    float2(mouseX, mouseY),
                    float2(pos.x, pos.y),
                    float2(sz.x, sz.y),
                    transformState,
                    verts,
                    commands);

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

            if (pressed && !draggingCamera && !transformState.active && !transformState.hovered) { // TODO: use hovered elsewhere?
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

                if (pressed && !draggingCamera && !transformState.active && mouseX >= screenPos.x - rad.x && mouseX <= screenPos.x + rad.x && mouseY >= screenPos.y - rad.y && mouseY <= screenPos.y + rad.y) {
                    selectedInstance = nullptr;
                    selectedLight = light;
                    selectedCamera = nullptr;
                }
            }

            if (!held)
                draggingCamera = false;

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
