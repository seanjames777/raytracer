#ifndef __SOLIDGEOM_H
#define __SOLIDGEOM_H

#include <math/vector.h>
#include <math/matrix.h>
#include <math/aabb.h>

#include <GL/glew.h>
#include <vector>

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

void drawWireBox(const AABB & bounds, const float4x4 & transform, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands);

void drawWireCircle(const float4x4 & transform, int numVertices, float r, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands);

void drawFilledCylinder(const float4x4 & transform, int numVertices, float r, float h, bool capTop, bool capBottom, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands);

void drawFilledCone(const float4x4 & transform, int numVertices, float r, float h, bool capBottom, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands);

void drawFilledQuad(const float4x4 & transform, float w, float h, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands);

void drawFilledSphere(const float4x4 & transform, int numVerticesX, int numVerticesY, float r, const float4 & color, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands);

void drawAxes(const float4x4 & transform, int numVertices, float lineRadius, float lineLength, float coneRadius, float coneLength, const float4 & xcolor, const float4 & ycolor, const float4 & zcolor, std::vector<SolidVertex> & vertices, std::vector<SolidDrawCmd> & commands);

#endif
