#ifndef __PREVIEW_H
#define __PREVIEW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <math/vector.h>
#include <math/matrix.h>
#include <image/image.h>
#include <core/scene.h>
#include <materials/pbrmaterial.h>
#include <light/pointlight.h>
#include <kdtree/kdtree.h>

#include <vector>
#include <unordered_map>

enum PVTextureType {
    PVTextureType2D = GL_TEXTURE_2D,
    PVTextureType2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
    PVTextureTypeCube = GL_TEXTURE_CUBE_MAP,
};

enum PVPixelFormat {
    PVPixelFormatRGBA8Unorm = GL_RGBA8,
    PVPixelFormatDepth32Float = GL_DEPTH_COMPONENT32F,
};

class PVTexture {
private:

    PVTextureType type;
    PVPixelFormat pixelFormat;
    int width;
    int height;
    int sampleCount;
    int mipmapLevelCount;
    GLuint texture;

    static std::unordered_map<std::string, PVTexture *> textures;

public:

    PVTexture(PVTextureType type, PVPixelFormat format, int width, int height, int sampleCount, int mipmapLevelCount);

    ~PVTexture();

    PVTextureType getType();

    PVPixelFormat getPixelFormat();

    int getWidth();

    int getHeight();

    int getSampleCount();

    int getMipmapLevelCount();

    GLuint getGLTexture();

    void bind(int index);

    void generateMipmaps();

    void setPixels(int slice, int level, int x, int y, int w, int h, void *data);

    static PVTexture *fromFile(const std::string & path, bool generateMipmaps);
};

enum PVFilter {
    PVFilterNone = 0,
    PVFilterNearest = GL_NEAREST,
    PVFilterLinear = GL_LINEAR,
};

enum PVAddress {
    PVAddressClamp = GL_CLAMP_TO_EDGE,
    PVAddressRepeat = GL_REPEAT
};

class PVSampler {
private:

    GLuint sampler;

    PVFilter minFilter;
    PVFilter magFilter;
    PVFilter mipFilter;
    PVAddress addressU;
    PVAddress addressV;
    int anisotropy;

public:

    PVSampler(PVFilter minFilter, PVFilter magFilter, PVFilter mipFilter, PVAddress addressU, PVAddress addressV, int anisotropy);

    ~PVSampler();

    PVFilter getMinFilter();

    PVFilter getMagFilter();

    PVFilter getMipFilter();

    PVAddress getAddressU();

    PVAddress getAddressV();

    int getAnisotropy();

    void bind(int index);
};

class PVBuffer {
private:

    GLuint buffer;

public:

    PVBuffer();

    ~PVBuffer();

    void setData(void *data, size_t length, GLenum usage);

    void bind(GLenum target);

    void bind(GLenum target, int index);
};

class PVShader {
private:

    GLuint program;

    std::unordered_map<const char *, GLint> uniformLocations;

public:

    PVShader(const char *vs_source, const char *fs_source, const GLchar **defines = nullptr, int num_defines = 0);

    ~PVShader();

    GLuint getUniformLocation(const char *uniform);

    void bind();

};

class PVFramebuffer {
private:

    GLuint fbo;
    GLuint color, depth;

    PVTexture *colorAttachments[4];
    PVTexture *depthAttachment;

public:

    PVFramebuffer();

    ~PVFramebuffer();

    void bind(GLenum target);

    void setColorAttachment(int index, PVTexture *texture);

    void setDepthAttachment(PVTexture *texture);

    void blit(int x, int y, int w, int h);

    void resolve(PVFramebuffer *target);

    PVTexture *getColorAttachment(int i);

    PVTexture *getDepthAttachment();

};

struct PVVertex {
    float position[3];
    float normal[3];
    float tangent[3];
    float uv[2];
};

enum PVRenderFlags {
    PVRenderFlagEnableTexturing     = (1 << 0),
    PVRenderFlagEnableNormalMapping = (1 << 1),
    PVRenderFlagEnableCulling       = (1 << 2),
    PVRenderFlagEnableLighting      = (1 << 3),
};

class PVMesh {
private:

    GLuint vao;
    int vertexCount;
    PVBuffer *vertexBuffer;
    KDTree kdTree;

public:

    std::string name;

public:

    bool intersect(const Ray & ray, Collision & result);

    PVMesh(util::vector<Triangle, 16> & triangles, bool reverseWinding, const std::string & name);

    ~PVMesh();

    void draw();

};

class PVMeshInstance {
public:

    PVMesh *mesh;
    float3 position;
    float3 rotation;
    float3 scale;
    float3 diffuseColor;
    float3 specularColor;
    float specularPower;
    std::string name;
    bool visible;
    PVTexture *diffuseTexture;
    PVTexture *normalTexture;

    PVMeshInstance(
        PVMesh *mesh,
        float3 diffuseColor,
        float3 specularColor,
        float specularPower,
        PVTexture *diffuseTexture,
        PVTexture *normalTexture,
        float3 position,
        float3 rotation,
        float3 scale,
        const std::string & name);

    ~PVMeshInstance();

    float4x4 getTransform();

    void draw(PVRenderFlags flags, PVShader *shader);

};

class PVLight {
public:

    float3 position;
    float intensity;
    float3 color;

public:

    PVLight(float3 position, float intensity, float3 color);

    ~PVLight();

    float3 getPosition() const;

    float getIntensity() const;

    float3 getColor() const;

};

enum PVFilterMode {
    PVFilterModeNearest = 0,
    PVFilterModeBilinear = 1,
    PVFilterModeTrilinear = 2,
    PVFilterModeAnisotropic = 3
};

class PVScene {
private:

    std::vector<PVMeshInstance *> instances;
    std::vector<PVLight *> lights;

    PVShader *depthShader;
    PVShader *lightingShader;

    Camera *camera;

    PVSampler *meshSampler;

    void addMesh(const MeshInstance *instance);

    void drawPass(PVRenderFlags flags,
                  PVShader *shader,
                  float4x4 viewMatrix,
                  float4x4 projectionMatrix,
                  float3 viewPosition,
                  PVFramebuffer *framebuffer);

public:

    int getNumMeshInstances();

    PVMeshInstance *getMeshInstance(int idx);

    int getNumLights();

    PVLight *getLight(int idx);

    int getNumCameras();

    Camera *getCamera(int idx);

    PVScene(Scene *scene);

    ~PVScene();

    void draw(PVRenderFlags flags,
              int maxLights,
              PVFilterMode filterMode,
              int anisotropy,
              float4x4 viewMatrix,
              float4x4 projectionMatrix,
              float3 viewPosition,
              PVFramebuffer *framebuffer);

};

#endif
