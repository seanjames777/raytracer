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

#ifdef DEBUG
#define GLCHECK(stmt)                                                             \
    stmt;                                                                         \
    {                                                                             \
        GLenum error = glGetError();                                              \
        if (error != GL_NO_ERROR) {                                               \
            std::cout << "GL Error check failed:" << std::endl;                   \
            std::cout << "    At: " << __FILE__ << ":" << __LINE__ << std::endl;  \
            std::cout << " Error: " << getErrorString(error) << std::endl;        \
            __builtin_trap();                                                     \
            assert(0);                                                            \
        }                                                                         \
    }
#else
#define GLCHECK(stmt) stmt;
#endif

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

    void bind(GLenum target, int index, int offset, int length);
};

enum PVVertexFormat {
    PVVertexFormatFloat = GL_FLOAT
};

class PVVertexArray {
private:

    GLuint vao;

public:

    PVVertexArray();

    ~PVVertexArray();

    void bind();

    void setVertexBuffer(PVBuffer *buffer);

    void setIndexBuffer(PVBuffer *buffer);

    void setVertexAttribute(int index, int components, PVVertexFormat format, bool normalized, int stride, int offset);

};

class PVShader {
private:

    GLuint program;

    std::unordered_map<const char *, GLint> uniformLocations;

public:

    PVShader(const char *vs_source, const char *fs_source, const GLchar **defines = nullptr, int num_defines = 0);

    ~PVShader();

    GLuint getUniformLocation(const char *uniform);

    void bindUniformBlock(const char *block, int binding);

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

    void getDimensions(int & width, int & height, int & sampleCount);

};

enum PVRenderFlags {
    PVRenderFlagEnableTexturing     = (1 << 0),
    PVRenderFlagEnableNormalMapping = (1 << 1),
    PVRenderFlagEnableCulling       = (1 << 2),
    PVRenderFlagEnableLighting      = (1 << 3),
    PVRenderFlagWireframe           = (1 << 4),
};

class PVMesh {
private:

    int indexCount;
    PVVertexArray *vertexArray;
    PVBuffer *vertexBuffer;
    PVBuffer *indexBuffer;
    KDTree kdTree;
    AABB bounds;

public:

    std::string name;

public:

    bool intersect(const Ray & ray, Collision & result);

    PVMesh(std::vector<PVVertex> & vertices, std::vector<uint32_t> & indices, bool reverseWinding, const std::string & name);

    ~PVMesh();

    void draw();

    const AABB & getBounds() const;

};

struct PVCameraUniforms {
    float4x4 viewProjection;
    float3   viewPosition;
};

struct PVLightUniforms {
    float3   lightPositions[4];
    float3   lightColors[4];
    int      numLights;
    int      enableLighting;
};

struct PVMeshInstanceUniforms {
    float4x4 transform;
    float3x3 orientation;
    float3   diffuseColor;
    float3   specularColor;
    float    specularPower;
    float    normalMapScale;
    int      hasDiffuseTexture; // TODO: pack the integers
    int      hasNormalTexture;
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
    float normalMapScale;
    std::string name;
    bool visible;
    PVTexture *diffuseTexture;
    PVTexture *normalTexture;

    PVMeshInstance(
        PVMesh *mesh,
        float3 diffuseColor,
        float3 specularColor,
        float specularPower,
        float normalMapScale,
        PVTexture *diffuseTexture,
        PVTexture *normalTexture,
        float3 position,
        float3 rotation,
        float3 scale,
        const std::string & name);

    ~PVMeshInstance();

    float4x4 getTransform();

    void updateUniforms(PVRenderFlags flags, PVMeshInstanceUniforms & uniforms);

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

    PVBuffer *cameraUniforms;
    PVBuffer *lightUniforms;
    PVBuffer *meshInstanceUniforms;

    void addMesh(const MeshInstance *instance);

    void drawView(PVRenderFlags flags,
                  PVShader *shader,
                  float4x4 viewMatrix,
                  float4x4 projectionMatrix,
                  float3 viewPosition);

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
              float3 viewPosition);

};

#endif
