#include <preview/preview.h>

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

#define GLCHECK() {                                                           \
    GLenum error = glGetError();                                              \
    if (error != GL_NO_ERROR) {                                               \
        std::cout << "GL Error check failed:" << std::endl;                   \
        std::cout << "    At: " << __FILE__ << ":" << __LINE__ << std::endl;  \
        std::cout << " Error: " << getErrorString(error) << std::endl;        \
        getchar();                                                            \
        exit(-1);                                                             \
    }                                                                         \
}

PVTexture::PVTexture(Image<float, 4> *image) {
    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float *pixels = new float[image->getWidth() * image->getHeight() * 4];
    
    image->getPixels(pixels);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, image->getWidth(), image->getHeight(), 0, GL_RGBA,
        GL_FLOAT, pixels);

    glGenerateMipmap(GL_TEXTURE_2D);

    delete [] pixels;

    glBindTexture(GL_TEXTURE_2D, 0);
}

PVTexture::~PVTexture() {
    glDeleteTextures(1, &texture);
}

void PVTexture::bind(int index) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture);
}

PVShader::PVShader(const char *vs_source, const char *fs_source) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint ps = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();

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

    glAttachShader(program, vs);
    glAttachShader(program, ps);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length > 0){
        std::vector<char> message(length+1);
        glGetProgramInfoLog(program, length, NULL, &message[0]);
        printf("%s\n", &message[0]);
    }

    glDeleteShader(vs);
    glDeleteShader(ps);
}

PVShader::~PVShader() {
    glDeleteProgram(program);
}

GLuint PVShader::getUniformLocation(const char *uniform) {
    return glGetUniformLocation(program, uniform);
}

void PVShader::bind() {
    glUseProgram(program);
}

PVMesh::PVMesh(
    int vertexOffset,
    int vertexCount,
    float3 diffuseColor,
    PVTexture *diffuseTexture,
    PVTexture *normalTexture)
    : vertexOffset(vertexOffset),
      vertexCount(vertexCount),
      diffuseColor(diffuseColor),
      diffuseTexture(diffuseTexture),
      normalTexture(normalTexture)
{
}

PVMesh::~PVMesh() {
}

void PVMesh::draw(PVShader *shader) {
    GLuint diffuseTextureLocation = shader->getUniformLocation("diffuseTexture");
    GLuint hasDiffuseTextureLocation = shader->getUniformLocation("hasDiffuseTexture");
    GLuint normalTextureLocation = shader->getUniformLocation("normalTexture");
    GLuint hasNormalTextureLocation = shader->getUniformLocation("hasNormalTexture");
    GLuint diffuseColorLocation = shader->getUniformLocation("diffuseColor");

    glUniform1i(diffuseTextureLocation, 0);
    glUniform1i(normalTextureLocation, 1);

    if (diffuseTexture) {
        diffuseTexture->bind(0);
        glUniform1i(hasDiffuseTextureLocation, 1);
    }
    else
        glUniform1i(hasDiffuseTextureLocation, 0);

    if (normalTexture) {
        normalTexture->bind(1);
        glUniform1i(hasNormalTextureLocation, 1);
    }
    else
        glUniform1i(hasNormalTextureLocation, 0);

    glUniform3f(diffuseColorLocation, diffuseColor.x, diffuseColor.y, diffuseColor.z);

    glDrawArrays(GL_TRIANGLES, vertexOffset, vertexCount);
}

PVLight::PVLight(float3 position, float3 color)
    : position(position),
      color(color)
{
}

PVLight::~PVLight() {
}

float3 PVLight::getPosition() const {
    return position;
}

float3 PVLight::getColor() const {
    return color;
}

const char *vs3d_source =
    "#version 410 core\n"
    "layout(location=0) in vec3 in_position;\n"
    "layout(location=1) in vec3 in_normal;\n"
    "layout(location=2) in vec3 in_tangent;\n"
    "layout(location=3) in vec2 in_uv;\n"
    "uniform mat4 viewProjection;\n"
    "out vec3 var_position;\n"
    "out vec3 var_normal;\n"
    "out vec3 var_tangent;\n"
    "out vec2 var_uv;\n"
    "void main() {\n"
    "    gl_Position = viewProjection * vec4(in_position, 1);\n"
    "    var_position = in_position;\n"
    "    var_normal = in_normal;\n"
    "    var_tangent = in_tangent;\n"
    "    var_uv = in_uv;\n"
    "}\n";

const char *fs3d_source =
    "#version 410 core\n"
    "in vec3 var_position;\n"
    "in vec3 var_normal;\n"
    "in vec3 var_tangent;\n"
    "in vec2 var_uv;\n"
    "uniform sampler2D diffuseTexture;\n"
    "uniform bool hasDiffuseTexture;\n"
    "uniform sampler2D normalTexture;\n"
    "uniform bool hasNormalTexture;\n"
    "uniform vec3 diffuseColor;\n"
    "uniform vec3 lightPosition[4];\n"
    "uniform vec3 lightColor[4];\n"
    "uniform int numLights;\n"
    "out vec4 out_color;\n"
    "void main() {\n"
    "    vec3 normal = normalize(var_normal);\n"
    "    vec3 tangent = normalize(var_tangent);\n"
    "    vec3 bitangent = cross(tangent, normal);\n"
    "    if (hasNormalTexture) {\n"
    "        vec3 tbn = texture(normalTexture, var_uv).rgb * 2.0 - 1.0;\n"
    "        normal = normalize(tbn.x * tangent + tbn.y * bitangent + tbn.z * normal);\n"
    "    }\n"
    "    vec3 lighting = vec3(0, 0, 0);\n"
    "    for (int i = 0; i < numLights; i++) {\n"
    "        vec3 l = lightPosition[i] - var_position;\n"
    "        float r = length(l);\n"
    "        l /= r;\n"
    "        r = max(r, 0.001);\n"
    "        float ndotl = clamp(dot(normal, l), 0, 1) / (r * r);\n"
    "        lighting += ndotl * lightColor[i];\n"
    "    }\n"
    "    vec3 diffuse = diffuseColor;\n"
    "    if (hasDiffuseTexture) diffuse = texture(diffuseTexture, var_uv).rgb;\n"
    "    out_color = vec4(lighting * diffuse, 1);\n"
    "}\n";

void PVScene::addMesh(Scene *scene, int materialID, int vertexOffset, int vertexCount) {
    PBRMaterial *material = (PBRMaterial *)scene->getMaterial(materialID);

    PVTexture *diffuseTexture = nullptr;
    PVTexture *normalTexture = nullptr;

    if (Image<float, 4> *diffuseImage = material->getDiffuseTexture())
        diffuseTexture = new PVTexture(diffuseImage);
    
    if (Image<float, 4> *normalImage = material->getNormalTexture())
        normalTexture = new PVTexture(normalImage);

    PVMesh *mesh = new PVMesh(
        vertexOffset,
        vertexCount,
        material->getDiffuseColor(),
        diffuseTexture,
        normalTexture);

    meshes.push_back(mesh);
}

PVScene::PVScene(Scene *scene) {
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    GLCHECK();

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    GLCHECK();

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, 64, (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 64, (void *)16);
    glVertexAttribPointer(2, 3, GL_FLOAT, false, 64, (void *)32);
    glVertexAttribPointer(3, 2, GL_FLOAT, false, 64, (void *)48);

    GLCHECK();

    std::vector<Vertex> vertices;

    int vertexCount = 0;
    int vertexOffset = 0;
    int curr_material = scene->getTriangles()[0].material_id;

    for (Triangle & tri : scene->getTriangles()) {
        if (curr_material != tri.material_id) {
            addMesh(scene, curr_material, vertexOffset, vertexCount);

            vertexOffset += vertexCount;
            vertexCount = 0;
            curr_material = tri.material_id;
        }

        vertices.push_back(tri.v[0]);
        vertices.push_back(tri.v[1]);
        vertices.push_back(tri.v[2]);

        vertexCount += 3;
    }

    addMesh(scene, curr_material, vertexOffset, vertexCount);

    Camera *camera = scene->getCamera();
    viewMatrix = lookAtLH(camera->getPosition(), camera->getTarget(), camera->getUp());
    projectionMatrix = perspectiveLH(camera->getFOV(), camera->getAspectRatio(), 0.1f, 100.0f);

    GLCHECK();

    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        &vertices[0],
        GL_DYNAMIC_DRAW);

    GLCHECK();

    glBindVertexArray(0);

    GLCHECK();

    for (int i = 0; i < scene->getNumLights(); i++) {
        PointLight *light = (PointLight *)scene->getLight(i);

        lights.push_back(new PVLight(light->getPosition(), light->getRadiance()));
    }

    shader = new PVShader(vs3d_source, fs3d_source);

    GLCHECK();
}

PVScene::~PVScene() {
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteBuffers(1, &vertexBuffer);

    delete shader;

    for (PVMesh *mesh : meshes)
        delete mesh;

    for (PVLight *light : lights)
        delete light;
}

void PVScene::draw(int x, int y, int w, int h) {
    static float time = 0.0f;
    time += 0.01f;
    viewMatrix = lookAtLH(float3(cosf(time) * 5, 2, sinf(time) * 5), float3(0, 0, 0), float3(0, 1, 0));
    float4x4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vertexArray);
    shader->bind();

    GLuint viewProjectionLocation = shader->getUniformLocation("viewProjection");
    glUniformMatrix4fv(viewProjectionLocation, 1, true, &viewProjectionMatrix.m[0][0]);

    GLuint lightColorLocation = shader->getUniformLocation("lightColor");
    GLuint lightPositionLocation = shader->getUniformLocation("lightPosition");
    GLuint numLightsLocation = shader->getUniformLocation("numLights");

    float lightPositions[3 * 4];
    float lightColors[3 * 4];

    int numLights = lights.size();

    if (numLights > 4)
        numLights = 4;

    for (int i = 0; i < numLights; i++) {
        PVLight *light = lights[i];

        float3 lightPosition = light->getPosition();
        float3 lightColor = light->getColor();

        lightPositions[i * 3 + 0] = lightPosition.x;
        lightPositions[i * 3 + 1] = lightPosition.y;
        lightPositions[i * 3 + 2] = lightPosition.z;

        lightColors[i * 3 + 0] = lightColor.x;
        lightColors[i * 3 + 1] = lightColor.y;
        lightColors[i * 3 + 2] = lightColor.z;
    }

    glUniform3fv(lightPositionLocation, numLights, lightPositions);
    glUniform3fv(lightColorLocation, numLights, lightColors);
    glUniform1i(numLightsLocation, numLights);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glViewport(x, y, w, h);

    for (PVMesh *mesh : meshes)
        mesh->draw(shader);

    glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(0);
    glBindVertexArray(0);

    GLCHECK();
}
