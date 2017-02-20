#include <preview/solidgeom.h>

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