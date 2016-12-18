#ifndef __PREVIEW_H
#define __PREVIEW_H

#include <GL/glew.h>
#include <glfw/glfw3.h>

#include <math/vector.h>
#include <math/matrix.h>
#include <image/image.h>
#include <core/scene.h>
#include <materials/pbrmaterial.h>
#include <light/pointlight.h>

#include <vector>

class PVTexture {
private:

    GLuint texture;

public:

    PVTexture(Image<float, 4> *image);

    ~PVTexture();

    void bind(int index);

};

class PVShader {
private:

    GLuint program;

public:

    PVShader(const char *vs_source, const char *fs_source);

    ~PVShader();

    GLuint getUniformLocation(const char *uniform);

    void bind();

};

class PVMesh {
private:

    int vertexOffset;
    int vertexCount;
    float3 diffuseColor;

    PVTexture *diffuseTexture;
    PVTexture *normalTexture;

public:

    PVMesh(
        int vertexOffset,
        int vertexCount,
        float3 diffuseColor,
        PVTexture *diffuseTexture,
        PVTexture *normalTexture);

    ~PVMesh();

    void draw(PVShader *shader);

};

class PVLight {
private:

    float3 position;
    float3 color;

public:

    PVLight(float3 position, float3 color);

    ~PVLight();

    float3 getPosition() const;

    float3 getColor() const;

};

class PVScene {
private:

    std::vector<PVMesh *> meshes;
    std::vector<PVLight *> lights;

    PVShader *shader;

    float4x4 viewMatrix;
    float4x4 projectionMatrix;

    GLuint vertexArray;
    GLuint vertexBuffer;

    void addMesh(Scene *scene, int materialID, int vertexOffset, int vertexCount);

public:

    PVScene(Scene *scene);

    ~PVScene();

    void draw(int x, int y, int w, int h);

};

#endif
