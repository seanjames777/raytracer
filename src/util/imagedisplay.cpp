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

GLFWwindow *window;
GLuint vao;
GLint viewSizeLocation;
double currTime;

PVShader *shader = nullptr;
PVBuffer *vertices = nullptr;
PVBuffer *indices = nullptr;

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
bool vsync;
int maxLights;
float scroll;

uint64_t frameIdx = 0;

#define MAX_FRAMES 120
float cpuFrameTimes[MAX_FRAMES];
float gpuFrameTimes[MAX_FRAMES];

#define MAX_QUERIES 3
GLuint queries[MAX_QUERIES];

float transform[16];

void setTransform(float3 position, float3 rotation, float3 scale) {
    ImGuizmo::RecomposeMatrixFromComponents(&position[0], &rotation[0], &scale[0], transform);
}

void getTransform(float3 & position, float3 & rotation, float3 & scale) {
    ImGuizmo::DecomposeMatrixToComponents(transform, &position[0], &rotation[0], &scale[0]);
}

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

    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

void imgui_render(ImDrawData *data) {
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

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

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

                glScissor((int)cmd->ClipRect.x, (int)(fbHeight - cmd->ClipRect.w), (int)(cmd->ClipRect.z - cmd->ClipRect.x), (int)(cmd->ClipRect.w - cmd->ClipRect.y));

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
#if 1
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

    glfwSwapInterval(1); // TODO: flag to enable

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

    currTime = glfwGetTime();

    for (int i = 0; i < MAX_FRAMES; i++)
        cpuFrameTimes[i] = 0.0f;

    for (int i = 0; i < MAX_FRAMES; i++)
        gpuFrameTimes[i] = 0.0f;

    culling = true;
    vsync = true;
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

struct ProfilerEventData {
    double cumulative;
    int hits;

    ProfilerEventData()
        : cumulative(0),
          hits(0)
    {
    }
};

struct ProfilerEvent {
    bool begin;
    const char *name;
    double time;
    ProfilerEventData *data;
};

std::vector<ProfilerEvent> profiler_events;
double profiler_frame_start, profiler_frame_end;
bool profiler_capture = false;

void profiler_begin_frame() {
    if (profiler_capture) {
        Timer timer;
        profiler_frame_start = timer.getTime();
    }
}

void profiler_end_frame() {
    if (profiler_capture) {
        Timer timer;
        profiler_frame_end = timer.getTime();
    }
}

void profiler_push(const char *name) {
    if (profiler_capture) {
        Timer timer;

        ProfilerEvent event;
        event.begin = true;
        event.name = name;
        event.time = timer.getTime();

        profiler_events.push_back(event);
    }
}

void profiler_pop() {
    if (profiler_capture) {
        Timer timer;

        ProfilerEvent event;
        event.begin = false;
        event.name = NULL;
        event.time = timer.getTime();

        profiler_events.push_back(event);
    }
}

void profiler_draw() {
#if 0
    // TODO: smooth out averages instead
    //if (ImGui::Button("Capture")) {

    //if (frameIdx % 10 == 0) {
        profiler_capture = true;
        //profiler_events.clear();
        //return;
    //}

    //profiler_capture = false;

    ImGuiWindow *window = ImGui::GetCurrentWindow();

    if (window->SkipItems)
        return;

    ImGuiIO& io = ImGui::GetIO();
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 orig = ImGui::GetCursorScreenPos();
    ImVec2 sz;
    sz.x = ImGui::GetContentRegionMax().x - window->WindowPadding.x;
    sz.y = g.FontSize + style.FramePadding.y * 2;

    ImRect bb(orig, orig + sz);

    if (!ImGui::ItemAdd(bb, NULL))
        return;

    //double frameTime = profiler_frame_end - profiler_frame_start;
    double frameTime = 50.0;
    float width = bb.Max.x - bb.Min.x;

    for (int i = 0; i <= 50; i++) {
        float x = (float)i / frameTime * width;

        window->DrawList->AddLine(ImVec2(bb.Min.x + x, bb.Min.y), ImVec2(bb.Min.x + x, bb.Max.y), ImColor(255, 255, 255, 255));

        char str[64];
        memset(str, 0, 64);
        sprintf(str, "%d", i);

        ImGui::RenderText(ImVec2(bb.Min.x + x, bb.Min.y), str);
    }

    struct ProfilerFrame {
        const char *name;
        double startTime;
    };

    std::vector<ProfilerFrame> stack;
    int depth = 0;

    for (ProfilerEvent & event : profiler_events) {
        if (event.begin) {
            ProfilerFrame frame;
            frame.name = event.name;
            frame.startTime = event.time;

            stack.push_back(frame);
            depth++;
        }
        else {
            ProfilerFrame & frame = stack.back();
            double endTime = event.time;

            float t0 = (frame.startTime - profiler_frame_start) / frameTime;
            float t1 = (endTime - profiler_frame_start) / frameTime;

            ImRect rect(ImVec2(bb.Min.x + t0 * width, bb.Min.y + 30 * depth), ImVec2(bb.Min.x + t1 * width, bb.Max.y + 30 * depth));

            window->DrawList->AddRectFilled(rect.Min, rect.Max, ImColor(255, 0, 0, 255));
            // TODO: just use strlen
            char value_buf[64];
            const char *value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), "%s", frame.name);
            ImGui::RenderTextClipped(rect.Min, rect.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));
            window->DrawList->AddRect(rect.Min, rect.Max, ImColor(255, 255, 0, 255));

            stack.pop_back();
            depth--;
        }
    }
#endif
    
    profiler_events.clear();
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

    float3 targ = scene->getCamera(0)->getTarget();
    float3 pos = scene->getCamera(0)->getPosition() - targ;

    float r = length(pos);
    float theta = atan2(pos.z, pos.x);
    float phi = acos(pos.y / r);

    if (!ImGuizmo::IsUsing() && !io.WantCaptureMouse) {
        if (mouseDown && prevMouseDown) {
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
                targ = targ + deltaMouseX * -0.03f * scene->getCamera(0)->getRight()
                            + deltaMouseY *  0.03f * scene->getCamera(0)->getUp();
            }
        }

        r += scroll * -0.1f;

        if (r < 0.001f)
            r = 0.001f;
    }

    pos.x = r * cos(theta) * sin(phi);
    pos.y = r * cos(phi);
    pos.z = r * sin(theta) * sin(phi);

    scene->getCamera(0)->setTarget(targ);
    scene->getCamera(0)->setPosition(pos + targ);

    //glFlush();
    Timer timer;

    profiler_begin_frame();
    profiler_push("Draw Scene");

#define ENABLE_GPU_PROFILING 1

#if ENABLE_GPU_PROFILING
    glBeginQuery(GL_TIME_ELAPSED, queries[frameIdx % 3]);
#endif

    profiler_push("Clear");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    profiler_pop();

    int width, height, windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &width, &height);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    int drawW = width / 2;
    int drawH = height / 2;

    if (colorTexture->getWidth() != drawW || colorTexture->getHeight() != drawH || colorTexture->getSampleCount() != sampleCount) {
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
            colorTexture = new PVTexture(PVTextureType2DMultisample, PVPixelFormatRGBA8Unorm, drawW, drawH, sampleCount, 1);
            depthTexture = new PVTexture(PVTextureType2DMultisample, PVPixelFormatDepth32Float, drawW, drawH, sampleCount, 1);

            resolveTexture = new PVTexture(PVTextureType2D, PVPixelFormatRGBA8Unorm, drawW, drawH, 0, 1);

            resolveFramebuffer = new PVFramebuffer();
            resolveFramebuffer->setColorAttachment(0, resolveTexture);
        }
        else {
            colorTexture = new PVTexture(PVTextureType2D, PVPixelFormatRGBA8Unorm, drawW, drawH, 0, 1);
            depthTexture = new PVTexture(PVTextureType2D, PVPixelFormatDepth32Float, drawW, drawH, 0, 1);
        }

        framebuffer = new PVFramebuffer();
        framebuffer->setColorAttachment(0, colorTexture);
        framebuffer->setDepthAttachment(depthTexture);
    }

    float4x4 viewMatrix = lookAtLH(scene->getCamera(0)->getPosition(), scene->getCamera(0)->getTarget(), scene->getCamera(0)->getUp());
    float4x4 projectionMatrix = perspectiveLH(scene->getCamera(0)->getFOV(), (float)width / (float)height, 0.1f, 100.0f);
    float3 viewPosition = scene->getCamera(0)->getPosition();

    PVRenderFlags flags = (PVRenderFlags)(
        (texturing ? PVRenderFlagEnableTexturing : 0) |
        (normalMapping ? PVRenderFlagEnableNormalMapping : 0) |
        (culling ? PVRenderFlagEnableCulling : 0) |
        (lighting ? PVRenderFlagEnableLighting : 0)
    );

    scene->draw(flags, maxLights, filterMode, anisotropy, viewMatrix, projectionMatrix, viewPosition, framebuffer);

    if (sampleCount > 0) {
        framebuffer->resolve(resolveFramebuffer);
        resolveFramebuffer->blit(0, 0, width, height);
    }
    else
        framebuffer->blit(0, 0, width, height);

#if ENABLE_GPU_PROFILING
    glEndQuery(GL_TIME_ELAPSED);
#endif

    profiler_pop();
    profiler_end_frame();

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

    io.DisplaySize.x = windowWidth;
    io.DisplaySize.y = windowHeight;
    io.DisplayFramebufferScale.x = (float)width / (float)windowWidth;
    io.DisplayFramebufferScale.y = (float)height / (float)windowHeight;
    io.DeltaTime = elapsed;
    io.MousePos = ImVec2(newMouseX, newMouseY);
    io.MouseWheel = scroll * 0.5f;

    scroll = 0.0f;

    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    {
        ImGui::SetNextWindowPos(ImVec2(350, 0), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Renderer  Settings", nullptr, ImVec2(440, 0));

        //glFlush();
        for (int i = 0; i < MAX_FRAMES - 1; i++)
            cpuFrameTimes[i] = cpuFrameTimes[i + 1];
        cpuFrameTimes[MAX_FRAMES - 1] = /*timer.getElapsedMilliseconds();*/ elapsed * 1000.0f;

        // TODO: Not really accurate
        for (int i = 0; i < MAX_FRAMES - 1; i++)
            gpuFrameTimes[i] = gpuFrameTimes[i + 1];
        gpuFrameTimes[MAX_FRAMES - 1] = gpuFrameTime;

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

        ImGui::SliderInt("MSAA Samples", &sampleCount, 0, maxSampleCount);

        ImGui::End();
    }

    float4x4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    if (!ImGuizmo::IsUsing() && !io.WantCaptureMouse && mouseDown && !prevMouseDown) {
        float2 ndc(mouseX, mouseY);
        ndc = ndc / float2(windowWidth, windowHeight);
        ndc = ndc * 2.0f - 1.0f;
        ndc.y = -ndc.y;

        float4x4 invViewProjection = inverse(viewProjectionMatrix);

        float4 near(ndc, 0.0001f, 1.0f); // TODO: -1 or 0 ?
        float4 far(ndc, 0.9999f, 1.0f);

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
            setTransform(closestHit->position, closestHit->rotation, closestHit->scale);
            selectedCamera = nullptr;
        }
    }

    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

        ImGui::Begin("Overlay", NULL, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar |
                                                          ImGuiWindowFlags_NoResize |
                                                          ImGuiWindowFlags_NoScrollbar |
                                                          ImGuiWindowFlags_NoInputs |
                                                          ImGuiWindowFlags_NoSavedSettings |
                                                          ImGuiWindowFlags_NoFocusOnAppearing |
                                                          ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImDrawList *drawList = ImGui::GetWindowDrawList();

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
            screenPos = screenPos * float2(windowWidth, windowHeight);

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

            if (!ImGuizmo::IsUsing() && !io.WantCaptureMouse && mouseDown && !prevMouseDown && mouseX >= screenPos.x - rad.x && mouseX <= screenPos.x + rad.x && mouseY >= screenPos.y - rad.y && mouseY <= screenPos.y + rad.y) {
                selectedInstance = nullptr;
                selectedLight = light;
                setTransform(light->position, float3(0, 0, 0), float3(1, 1, 1));
                selectedCamera = nullptr;
            }
        }

        ImGui::End();
    }

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
                setTransform(selectedInstance->position, selectedInstance->rotation, selectedInstance->scale);
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
                setTransform(selectedLight->position, float3(0, 0, 0), float3(1, 1, 1));
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

    ImGui::SetNextWindowPos(ImVec2(windowWidth - 350, 0));
    ImGui::SetNextWindowSize(ImVec2(350, windowHeight));

    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (selectedInstance) {
        ImGui::ColorPicker("Diffuse Color", &selectedInstance->diffuseColor[0]);
        ImGui::ColorPicker("Specular Color", &selectedInstance->specularColor[0]);
        ImGui::DragFloat("Specular Power", &selectedInstance->specularPower, 0.5f, 1.0f, 64.0f);

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

        getTransform(selectedInstance->position, selectedInstance->rotation, selectedInstance->scale);
        ImGui::InputFloat3("Position", &selectedInstance->position[0], 3);
        ImGui::InputFloat3("Rotation", &selectedInstance->rotation[0], 3);
        ImGui::InputFloat3("Scale", &selectedInstance->scale[0], 3);

        float4x4 vt = transpose(viewMatrix);
        float4x4 pt = transpose(projectionMatrix);
        ImGuizmo::Manipulate(&vt.m[0][0], &pt.m[0][0], ImGuizmo::TRANSLATE, ImGuizmo::WORLD, transform);
    }
    else if (selectedLight) {
        ImGui::ColorPicker("Color", &selectedLight->color[0]);

        ImGui::DragFloat("Intensity", &selectedLight->intensity, 1.0f, 0.0f, 1000.0f);

        float3 rotation, scale;
        getTransform(selectedLight->position, rotation, scale);
        ImGui::InputFloat3("Position", &selectedLight->position[0], 3);

        float4x4 vt = transpose(viewMatrix);
        float4x4 pt = transpose(projectionMatrix);
        ImGuizmo::Manipulate(&vt.m[0][0], &pt.m[0][0], ImGuizmo::TRANSLATE, ImGuizmo::WORLD, transform);
    }
    else if (selectedCamera) {
        ImGui::DragFloat("FOV", &camFOV, 0.5f, 5.0f, 150.0f);
        selectedCamera->setFOV(camFOV / 180.0f * (float)M_PI);
    }

    ImGui::End();

    // TODO: add a way to select nothing

#if 0
    ImGui::Begin("Profiler");
    profiler_draw();
    ImGui::End();
#endif

    ImGui::Render();
}
