#include <preview/preview.h>
#include <util/timer.h>
#include <kdtree/kdsahbuilder.h>
#include <kdtree/kdmedianbuilder.h>

enum PVRenderFlagsPrivate {
    PVRenderFlagPrivateDepthOnly     = (1 << 5),
    PVRenderFlagPrivatePreserveDepth = (1 << 6)
};

GLenum getPixelFormatDataFormat(PVPixelFormat pixelFormat) {
    switch (pixelFormat) {
    case PVPixelFormatRGBA8Unorm:
        return GL_RGBA;
    case PVPixelFormatDepth32Float:
        return GL_DEPTH_COMPONENT;
    default:
        assert(0);
        break;
    }
}

GLenum getPixelFormatDataType(PVPixelFormat pixelFormat) {
    switch (pixelFormat) {
    case PVPixelFormatRGBA8Unorm:
        return GL_UNSIGNED_BYTE;
    case PVPixelFormatDepth32Float:
        return GL_FLOAT;
    default:
        assert(0);
        break;
    }
}

PVTexture::PVTexture(PVTextureType type, PVPixelFormat pixelFormat, int width, int height, int sampleCount, int mipmapLevelCount)
    : type(type),
      pixelFormat(pixelFormat),
      width(width),
      height(height),
      sampleCount(sampleCount),
      mipmapLevelCount(mipmapLevelCount)
{
    GLCHECK(glGenTextures(1, &texture));

    bind(0);

    switch (type) {
    case PVTextureType2D:
    case PVTextureTypeCube:
        GLCHECK(glTexStorage2D(type, mipmapLevelCount, pixelFormat, width, height));
        break;
    case PVTextureType2DMultisample:
        GLCHECK(glTexImage2DMultisample(type, sampleCount, pixelFormat, width, height, false));
        break;
    };
}

PVTexture::~PVTexture() {
    GLCHECK(glDeleteTextures(1, &texture));
}

PVTextureType PVTexture::getType() {
    return type;
}

PVPixelFormat PVTexture::getPixelFormat() {
    return pixelFormat;
}

int PVTexture::getWidth() {
    return width;
}

int PVTexture::getHeight() {
    return height;
}

int PVTexture::getSampleCount() {
    return sampleCount;
}

int PVTexture::getMipmapLevelCount() {
    return mipmapLevelCount;
}

GLuint PVTexture::getGLTexture() {
    return texture;
}

void PVTexture::bind(int index) {
    GLCHECK(glActiveTexture(GL_TEXTURE0 + index));
    GLCHECK(glBindTexture(type, texture));
}

void PVTexture::generateMipmaps() {
    bind(0);
    GLCHECK(glGenerateMipmap(type));

    // TODO: does this work for cubes?
}

void PVTexture::setPixels(int slice, int level, int x, int y, int w, int h, void *data) {
    bind(0);

    GLenum dataFormat = getPixelFormatDataFormat(pixelFormat);
    GLenum dataType = getPixelFormatDataType(pixelFormat);

    // TODO: use slice

    switch (type) {
    case PVTextureType2D:
    case PVTextureTypeCube:
        GLCHECK(glTexSubImage2D(type, level, x, y, w, h, dataFormat, dataType, data));
        break;
    case PVTextureType2DMultisample:
        assert(0);
        break;
    }
}

PVSampler::PVSampler(PVFilter minFilter, PVFilter magFilter, PVFilter mipFilter, PVAddress addressU, PVAddress addressV, int anisotropy)
    : minFilter(minFilter),
      magFilter(magFilter),
      mipFilter(mipFilter),
      addressU(addressU),
      addressV(addressV),
      anisotropy(anisotropy)
{
    GLCHECK(glGenSamplers(1, &sampler));

    switch (minFilter) {
    case PVFilterNone:
        assert(0);
        break;
    case PVFilterNearest:
        switch (mipFilter) {
        case PVFilterNone:
            GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            break;
        case PVFilterNearest:
            GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
            break;
        case PVFilterLinear:
            GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
            break;
        }
        break;
    case PVFilterLinear:
        switch (mipFilter) {
        case PVFilterNone:
            GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            break;
        case PVFilterNearest:
            GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
            break;
        case PVFilterLinear:
            GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
            break;
        }
        break;
    }

    GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, magFilter));
    GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, addressU));
    GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, addressV));
    GLCHECK(glSamplerParameteri(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy));
}

PVSampler::~PVSampler() {
    GLCHECK(glDeleteSamplers(1, &sampler));
}

PVFilter PVSampler::getMinFilter() {
    return minFilter;
}

PVFilter PVSampler::getMagFilter() {
    return magFilter;
}

PVFilter PVSampler::getMipFilter() {
    return mipFilter;
}

PVAddress PVSampler::getAddressU() {
    return addressU;
}

PVAddress PVSampler::getAddressV() {
    return addressV;
}

int PVSampler::getAnisotropy() {
    return anisotropy;
}

void PVSampler::bind(int index) {
    GLCHECK(glActiveTexture(GL_TEXTURE0 + index));
    GLCHECK(glBindSampler(index, sampler));
}

PVBuffer::PVBuffer() {
    GLCHECK(glGenBuffers(1, &buffer));
}

PVBuffer::~PVBuffer() {
    GLCHECK(glDeleteBuffers(1, &buffer));
}

void PVBuffer::setData(void *data, size_t length, GLenum usage) {
    bind(GL_UNIFORM_BUFFER);
    GLCHECK(glBufferData(GL_UNIFORM_BUFFER, length, data, usage));
}

void PVBuffer::bind(GLenum target) {
    GLCHECK(glBindBuffer(target, buffer));
}

void PVBuffer::bind(GLenum target, int index) {
    GLCHECK(glBindBufferBase(target, index, buffer));
}

void PVBuffer::bind(GLenum target, int index, int offset, int length) {
    GLCHECK(glBindBufferRange(target, index, buffer, offset, length));
}

class PVShaderFunction {
private:

    GLuint shader;

public:

    PVShaderFunction(GLenum type, const char *src_path, const GLchar **defines = nullptr, int num_defines = 0) {
        shader = GLCHECK(glCreateShader(type));

        FILE *fd = fopen(relToExeDir(src_path).c_str(), "r");

        fseek(fd, 0, SEEK_END);
        int src_len = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        char *src = (char *)malloc(src_len);
        fread(src, 1, src_len, fd);

        fclose(fd);

        const GLchar **sources = (const GLchar **)malloc(sizeof(GLchar *) * (num_defines + 1));
        GLint *lengths = (GLint *)malloc(sizeof(GLint) * (num_defines + 1));

        for (int i = 0; i < num_defines; i++) {
            sources[i] = defines[i];
            lengths[i] = strlen(defines[i]);
        }

        sources[num_defines] = src;
        lengths[num_defines] = src_len;

        GLCHECK(glShaderSource(shader, num_defines + 1, sources, lengths));

        free(sources);
        free(lengths);
        free(src);

        GLCHECK(glCompileShader(shader));

        GLint result = GL_FALSE;
        int length;

        GLCHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &result));
        GLCHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length));

        if (length > 0){
            char *message = (char *)malloc(length + 1);
            GLCHECK(glGetShaderInfoLog(shader, length, NULL, message));
            printf("%s\n", message);
            free(message);
        }

        assert(result);
    }

    ~PVShaderFunction() {
        GLCHECK(glDeleteShader(shader));
    }

    GLuint getGLShader() {
        return shader;
    }
};

PVShader::PVShader(const char *vs_path, const char *fs_path, const GLchar **defines, int num_defines) {
    program = GLCHECK(glCreateProgram());

    if (vs_path) {
        PVShaderFunction vs(GL_VERTEX_SHADER, vs_path, defines, num_defines);
        GLCHECK(glAttachShader(program, vs.getGLShader()));
    }

    if (fs_path) {
        PVShaderFunction fs(GL_FRAGMENT_SHADER, fs_path, defines, num_defines);
        GLCHECK(glAttachShader(program, fs.getGLShader()));
    }

    GLCHECK(glLinkProgram(program));

    GLint result = GL_FALSE;
    int length;

    GLCHECK(glGetProgramiv(program, GL_LINK_STATUS, &result));
    GLCHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
    if (length > 0){
        char *message = (char *)malloc(length + 1);
        GLCHECK(glGetProgramInfoLog(program, length, NULL, message));
        printf("%s\n", message);
            free(message);
    }

    assert(result);
}

PVShader::~PVShader() {
    GLCHECK(glDeleteProgram(program));
}

GLuint PVShader::getUniformLocation(const char *uniform) {
    if (uniformLocations.find(uniform) != uniformLocations.end())
        return uniformLocations[uniform];

    GLint location = GLCHECK(glGetUniformLocation(program, uniform));

    uniformLocations[uniform] = location;

    return location;
}

void PVShader::bind() {
    GLCHECK(glUseProgram(program));
}

void PVShader::bindUniformBlock(const char *block, int binding) {
    GLint index = GLCHECK(glGetUniformBlockIndex(program, block));

    if (index >= 0)
        GLCHECK(glUniformBlockBinding(program, index, binding));
}

PVFramebuffer::PVFramebuffer()
    : depthAttachment(nullptr)
{
    for (int i = 0; i < 4; i++)
        colorAttachments[i] = nullptr;

    GLCHECK(glGenFramebuffers(1, &fbo));
}

PVFramebuffer::~PVFramebuffer() {
    GLCHECK(glDeleteFramebuffers(1, &fbo));
}

void PVFramebuffer::bind(GLenum target) {
    GLCHECK(glBindFramebuffer(target, fbo));
}

void PVFramebuffer::resolve(PVFramebuffer *target) {
    bind(GL_READ_FRAMEBUFFER);
    target->bind(GL_DRAW_FRAMEBUFFER);

    // TODO: Only resolves first color attachment

    GLCHECK(glBlitFramebuffer(0, 0, colorAttachments[0]->getWidth(), colorAttachments[0]->getHeight(), 0, 0, colorAttachments[0]->getWidth(), colorAttachments[0]->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST));

    GLCHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void PVFramebuffer::blit(int x, int y, int w, int h) {
    bind(GL_READ_FRAMEBUFFER);
    // TODO: linear vs. nearest
    GLCHECK(glBlitFramebuffer(0, 0, colorAttachments[0]->getWidth(), colorAttachments[0]->getHeight(), x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_LINEAR));
}

void PVFramebuffer::setColorAttachment(int index, PVTexture *texture) {
    colorAttachments[index] = texture;

    bind(GL_FRAMEBUFFER);
    GLCHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture->getSampleCount() > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texture->getGLTexture(), 0));

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: Incomplete framebuffer\n");
        exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PVFramebuffer::setDepthAttachment(PVTexture *texture) {
    depthAttachment = texture;

    bind(GL_FRAMEBUFFER);
    GLCHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->getSampleCount() > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texture->getGLTexture(), 0));

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("Error: Incomplete framebuffer\n");
        exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PVTexture *PVFramebuffer::getColorAttachment(int i) {
    return colorAttachments[i];
}

PVTexture *PVFramebuffer::getDepthAttachment() {
    return depthAttachment;
}

void PVFramebuffer::getDimensions(int & width, int & height, int & sampleCount) {
    if (depthAttachment) {
        width = depthAttachment->getWidth();
        height = depthAttachment->getHeight();
        sampleCount = depthAttachment->getSampleCount();
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (colorAttachments[i]) {
                width = colorAttachments[i]->getWidth();
                height = colorAttachments[i]->getHeight();
                sampleCount = colorAttachments[i]->getSampleCount();
                break;
            }
        }
    }
}

PVVertexArray::PVVertexArray() {
    GLCHECK(glGenVertexArrays(1, &vao));
}

PVVertexArray::~PVVertexArray() {
    GLCHECK(glDeleteVertexArrays(1, &vao));
}

void PVVertexArray::bind() {
    GLCHECK(glBindVertexArray(vao));
}

void PVVertexArray::setVertexBuffer(PVBuffer *buffer) {
    GLCHECK(glBindVertexArray(vao));
    buffer->bind(GL_ARRAY_BUFFER);
    GLCHECK(glBindVertexArray(0));
}

void PVVertexArray::setIndexBuffer(PVBuffer *buffer) {
    GLCHECK(glBindVertexArray(vao));
    buffer->bind(GL_ELEMENT_ARRAY_BUFFER);
    GLCHECK(glBindVertexArray(0));
}

void PVVertexArray::setVertexAttribute(int index, int components, PVVertexFormat format, bool normalized, int stride, int offset) {
    GLCHECK(glBindVertexArray(vao));
    GLCHECK(glEnableVertexAttribArray(index));
    GLCHECK(glVertexAttribPointer(index, components, format, normalized, stride, (void *)offset));
    GLCHECK(glBindVertexArray(0));
}

Vertex createVertex(const PVVertex & vertex)
{
    float3 position = float3(vertex.position[0], vertex.position[1], vertex.position[2]);
    float3 normal = float3(vertex.normal[0], vertex.normal[1], vertex.normal[2]);
    float3 tangent = float3(vertex.tangent[0], vertex.tangent[1], vertex.tangent[2]);
    float2 uv = float2(vertex.uv[0], vertex.uv[1]);

    return Vertex(position, normalize(normal), normalize(tangent), uv);
}

PVMesh::PVMesh(
    std::vector<PVVertex> & vertices,
    std::vector<uint32_t> & indices,
    bool reverseWinding,
    const std::string & name)
    : indexCount(indices.size()),
      name(name)
{
    util::vector<Triangle, 16> triangles;

    bounds = AABB(float3(INFINITY, INFINITY, INFINITY), float3(-INFINITY, -INFINITY, -INFINITY));

    for (int j = 0; j < indices.size() / 3; j++) {
        int i0 = indices[j * 3 + 0];
        int i1 = indices[j * 3 + 1];
        int i2 = indices[j * 3 + 2];

        if (!reverseWinding) {
            Triangle tri(
                createVertex(vertices[i0]),
                createVertex(vertices[i1]),
                createVertex(vertices[i2]),
                triangles.size(),
                0
            );

            for (int k = 0; k < 3; k++)
                bounds.join(tri.v[k].position);

            triangles.push_back(tri);
        }
        else {
            Triangle tri(
                createVertex(vertices[i2]),
                createVertex(vertices[i1]),
                createVertex(vertices[i0]),
                triangles.size(),
                0
            );

            for (int k = 0; k < 3; k++)
                tri.v[k].normal = -tri.v[k].normal;

            for (int k = 0; k < 3; k++)
                bounds.join(tri.v[k].position);

            triangles.push_back(tri);
        }
    }

    KDSAHBuilder builder(kdTree, triangles, 12.0f, 1.0f);
    //KDMedianBuilder builder(kdTree, triangles);
    builder.build();

    vertexArray = new PVVertexArray();

    vertexBuffer = new PVBuffer();
    vertexArray->setVertexBuffer(vertexBuffer);
    vertexBuffer->setData(&vertices[0], sizeof(PVVertex) * vertices.size(), GL_STATIC_DRAW);

    indexBuffer = new PVBuffer();
    vertexArray->setIndexBuffer(indexBuffer);
    indexBuffer->setData(&indices[0], sizeof(uint32_t) * indices.size(), GL_STATIC_DRAW);

    // TODO: aligning to 4 bytes may actually help...
    vertexArray->setVertexAttribute(0, 3, PVVertexFormatFloat, false, 44, 0);
    vertexArray->setVertexAttribute(1, 3, PVVertexFormatFloat, false, 44, 12);
    vertexArray->setVertexAttribute(2, 3, PVVertexFormatFloat, false, 44, 24);
    vertexArray->setVertexAttribute(3, 2, PVVertexFormatFloat, false, 44, 36);
}

PVMesh::~PVMesh() {
    // TODO: delete stuff
}

const AABB & PVMesh::getBounds() const {
    return bounds;
}

void PVMesh::draw() {
    vertexArray->bind();
    GLCHECK(glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, NULL));
}

bool PVMesh::intersect(const Ray & ray, Collision & result) {
    return kdTree.intersect(ray, INFINITY, result);
}

// TODO: cache meshes, sort instances by mesh, etc.

PVMeshInstance::PVMeshInstance(
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
    const std::string & name)
    : mesh(mesh),
      diffuseColor(diffuseColor),
      specularColor(specularColor),
      normalMapScale(normalMapScale),
      specularPower(specularPower),
      diffuseTexture(diffuseTexture),
      normalTexture(normalTexture),
      position(position),
      rotation(rotation),
      scale(scale),
      name(name),
      visible(true)
{
}

PVMeshInstance::~PVMeshInstance() {
}

float4x4 PVMeshInstance::getTransform() {
    return translation(position) * ::rotation(rotation) * ::scale(scale);
}

void PVMeshInstance::updateUniforms(PVRenderFlags flags, PVMeshInstanceUniforms & uniforms) {
    uniforms.diffuseColor = diffuseColor;
    uniforms.specularColor = specularColor;
    uniforms.specularPower = specularPower;
    uniforms.normalMapScale = normalMapScale;

    float4x4 transform = getTransform();
    // TODO: doubly transposed
    float3x3 orientation = upper3x3(transpose(inverse(transform)));

    uniforms.transform = transpose(transform);
    uniforms.orientation = transpose(orientation);

    uniforms.hasDiffuseTexture = (flags & PVRenderFlagEnableTexturing) != 0 && diffuseTexture != nullptr;
    uniforms.hasNormalTexture = (flags & PVRenderFlagEnableNormalMapping) != 0 && normalTexture != nullptr;
}

void PVMeshInstance::draw(PVRenderFlags flags, PVShader *shader) {
    GLint diffuseTextureLocation = shader->getUniformLocation("diffuseTexture");
    GLint normalTextureLocation = shader->getUniformLocation("normalTexture");

    GLCHECK(glUniform1i(diffuseTextureLocation, 0));
    GLCHECK(glUniform1i(normalTextureLocation, 1));

    if ((flags & PVRenderFlagEnableTexturing) != 0 && diffuseTexture)
        diffuseTexture->bind(0);

    if ((flags & PVRenderFlagEnableNormalMapping) != 0 && normalTexture)
        normalTexture->bind(1);

    mesh->draw();
}

PVLight::PVLight(float3 position, float intensity, float3 color)
    : position(position),
      intensity(intensity),
      color(color)
{
}

PVLight::~PVLight() {
}

float3 PVLight::getPosition() const {
    return position;
}

float PVLight::getIntensity() const {
    return intensity;
}

float3 PVLight::getColor() const {
    return color;
}

std::unordered_map<std::string, PVTexture *> PVTexture::textures;

int ilog2(int x) {
    int log = 0;

    while (x > 1) {
        log++;
        x >>= 1;
    }

    return log;
}

PVTexture *PVTexture::fromFile(const std::string & path, bool generateMipmaps) {
    std::string filename = relToExeDir(path);

    // TODO: cache does not take mipmaps into account
    auto kv = textures.find(filename);

    if (kv != textures.end())
        return kv->second;
    
    Image<float, 4> *image = ImageLoader::load(filename);

    float *f32 = new float[image->getWidth() * image->getHeight() * 4];
    image->getPixels(f32);

    delete image;

    uint8_t *i8 = new uint8_t[image->getWidth() * image->getHeight() * 4];

    for (int i = 0; i < image->getWidth() * image->getHeight() * 4; i++)
        i8[i] = (uint8_t)(f32[i] * 255.0f);

    delete [] f32;

    int width = image->getWidth();
    int height = image->getHeight();

    int maxDim = max(width, height);
    int levels = ilog2(maxDim) + 1;

    if (!generateMipmaps)
        levels = 1;

    PVTexture *texture = new PVTexture(PVTextureType2D, PVPixelFormatRGBA8Unorm, width, height, 0, levels);
    texture->setPixels(0, 0, 0, 0, width, height, i8);

    delete [] i8;

    if (generateMipmaps)
        texture->generateMipmaps();

    textures[filename] = texture;

    return texture;
}

void PVScene::addMesh(const MeshInstance *instance) {
    Mesh *mesh = instance->mesh;

    for (int i = 0; i < mesh->getNumSubmeshes(); i++) {
        Submesh *submesh = mesh->getSubmesh(i);

        MaterialProperties *material = (MaterialProperties *)mesh->getMaterial(submesh->getMaterialID());

        PVTexture *diffuseTexture = nullptr;
        PVTexture *normalTexture = nullptr;

        if (material->diffuseTexture != "")
            diffuseTexture = PVTexture::fromFile("content/textures/" + material->diffuseTexture, true);

        if (material->normalTexture != "")
            normalTexture = PVTexture::fromFile("content/textures/" + material->normalTexture, true);

        PVMesh *pvmesh = new PVMesh(
            submesh->getVertices(),
            submesh->getIndices(),
            instance->reverseWinding,
            submesh->getName());

        PVMeshInstance *pvinstance = new PVMeshInstance(
            pvmesh,
            material->diffuseColor,
            float3(0.8f),
            16.0f,
            1.0f,
            diffuseTexture,
            normalTexture,
            instance->translation,
            instance->rotation,
            instance->scale,
            submesh->getName());

        instances.push_back(pvinstance);
    }
}

PVScene::PVScene(Scene *scene) {
    camera = scene->getCamera();

    for (int i = 0; i < scene->getNumMeshInstances(); i++)
        addMesh(scene->getMeshInstance(i));

    // std::sort(meshes.begin(), meshes.end(), [](PVMesh *l, PVMesh *r) {
    //     return l->getMaterialID() < r->getMaterialID();
    // });

    for (int i = 0; i < scene->getNumLights(); i++) {
        PointLight *light = (PointLight *)scene->getLight(i);

        float3 color = light->getRadiance();
        float intensity = length(color);
        color = color / intensity;

        lights.push_back(new PVLight(light->getPosition(), intensity, color));
    }

    const GLchar *defines[2];
    defines[0] = "#version 410 core\n";

    defines[1] = "#define SUPPORT_LIGHTING 1\n";
    lightingShader = new PVShader("pv_vertex.glsl", "pv_fragment.glsl", defines, sizeof(defines) / sizeof(defines[0]));

    defines[1] = "#define SUPPORT_LIGHTING 0\n";
    depthShader = new PVShader("pv_vertex.glsl", "pv_fragment.glsl", defines, sizeof(defines) / sizeof(defines[0]));

    lightingShader->bindUniformBlock("CameraUniforms", 0);
    lightingShader->bindUniformBlock("LightUniforms", 1);
    lightingShader->bindUniformBlock("MeshInstanceUniforms", 2);

    depthShader->bindUniformBlock("CameraUniforms", 0);
    depthShader->bindUniformBlock("LightUniforms", 1);
    depthShader->bindUniformBlock("MeshInstanceUniforms", 2);

    GLCHECK(glBindVertexArray(0));

    meshSampler = new PVSampler(PVFilterLinear, PVFilterLinear, PVFilterNearest, PVAddressRepeat, PVAddressRepeat, 1);

    cameraUniforms = new PVBuffer();
    lightUniforms = new PVBuffer();
    meshInstanceUniforms = new PVBuffer();
}

PVScene::~PVScene() {
    delete lightingShader;
    delete depthShader;
    
    for (PVMeshInstance *instance : instances)
        delete instance;

    // TODO: also need to delete the mesh

    for (PVLight *light : lights)
        delete light;
}

void PVScene::drawView(PVRenderFlags flags,
                       PVShader *shader,
                       float4x4 viewMatrix,
                       float4x4 projectionMatrix,
                       float3 viewPosition)
{
    if (flags & PVRenderFlagPrivateDepthOnly) {
        GLCHECK(glColorMask(false, false, false, false));
        //GLCHECK(glDrawBuffer(GL_NONE)); // TODO
    }

    shader->bind();

    // TODO: Don't fill out all these uniforms for depth only passes

    PVCameraUniforms cameraUniforms;
    cameraUniforms.viewProjection = transpose(projectionMatrix * viewMatrix);
    cameraUniforms.viewPosition = viewPosition;

    // TODO: static draw?
    this->cameraUniforms->setData(&cameraUniforms, sizeof(cameraUniforms), GL_STATIC_DRAW);

    // TODO: don't need to clear color without color attachments
    GLCHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCHECK(glClearDepth(1.0f));

    GLCHECK(glClear(GL_COLOR_BUFFER_BIT | ((flags & PVRenderFlagPrivatePreserveDepth ? 0 : GL_DEPTH_BUFFER_BIT))));

    GLCHECK(glEnable(GL_DEPTH_TEST));

    GLCHECK(glDepthFunc((flags & PVRenderFlagPrivatePreserveDepth) ? GL_EQUAL : GL_LESS));

    if (flags & PVRenderFlagEnableCulling)
        GLCHECK(glEnable(GL_CULL_FACE));

    if (flags & PVRenderFlagWireframe)
        GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

    GLCHECK(glCullFace(GL_BACK));
    GLCHECK(glFrontFace(GL_CW));

    for (int i = 0; i < 2; i++)
        meshSampler->bind(i);

    this->cameraUniforms->bind(GL_UNIFORM_BUFFER, 0);
    this->lightUniforms->bind(GL_UNIFORM_BUFFER, 1);

    int instanceUniformsSize = sizeof(PVMeshInstanceUniforms);
    instanceUniformsSize = (instanceUniformsSize + 255) & ~255;
    int instanceUniformsOffset = 0;

    for (PVMeshInstance *instance : instances)
        if (instance->visible) {
            this->meshInstanceUniforms->bind(GL_UNIFORM_BUFFER, 2, instanceUniformsOffset, instanceUniformsSize);
            instanceUniformsOffset += instanceUniformsSize;
            instance->draw(flags, shader);
        }

    GLCHECK(glBindVertexArray(0));

    //GLCHECK(glDrawBuffer(GL_BACK));
    GLCHECK(glColorMask(true, true, true, true));

    if (flags & PVRenderFlagWireframe)
        GLCHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    GLCHECK(glDisable(GL_CULL_FACE));

    GLCHECK(glDepthFunc(GL_LESS));

    GLCHECK(glDisable(GL_DEPTH_TEST));

    GLCHECK(glUseProgram(0));
}

void PVScene::draw(PVRenderFlags flags,
                   int maxLights,
                   PVFilterMode filterMode,
                   int anisotropy,
                   float4x4 viewMatrix,
                   float4x4 projectionMatrix,
                   float3 viewPosition)
{
    PVFilter minFilter, magFilter, mipFilter;

    switch (filterMode) {
    case PVFilterModeNearest:
        anisotropy = 1;
        minFilter = PVFilterNearest;
        magFilter = PVFilterNearest;
        mipFilter = PVFilterNearest;
        break;
    case PVFilterModeBilinear:
        anisotropy = 1;
        minFilter = PVFilterLinear;
        magFilter = PVFilterLinear;
        mipFilter = PVFilterNearest;
        break;
    case PVFilterModeTrilinear:
        anisotropy = 1;
    case PVFilterModeAnisotropic:
        minFilter = PVFilterLinear;
        magFilter = PVFilterLinear;
        mipFilter = PVFilterLinear;
        break;
    // TODO: DontCare for depth passes
    }

    if (meshSampler->getMinFilter() != minFilter || meshSampler->getMagFilter() != magFilter || meshSampler->getMipFilter() != mipFilter || meshSampler->getAnisotropy() != anisotropy) {
        delete meshSampler;
        meshSampler = new PVSampler(minFilter, magFilter, mipFilter, PVAddressRepeat, PVAddressRepeat, anisotropy);
    }

    PVShader *shader = nullptr;

    if (flags & PVRenderFlagPrivateDepthOnly)
        shader = depthShader;
    else
        shader = lightingShader;

    PVLightUniforms lightUniforms;

    int numLights = lights.size();

    if (numLights > maxLights)
        numLights = maxLights;

    lightUniforms.numLights = numLights;

    for (int i = 0; i < numLights; i++) {
        PVLight *light = lights[i];

        lightUniforms.lightPositions[i] = light->getPosition();
        lightUniforms.lightColors[i] = light->getColor() * light->getIntensity();
    }

    lightUniforms.enableLighting = (flags & PVRenderFlagEnableLighting) != 0;

    int numVisibleMeshes = 0;

    for (PVMeshInstance *instance : instances)
        if (instance->visible)
            numVisibleMeshes++;

    int instanceUniformsSize = sizeof(PVMeshInstanceUniforms);
    instanceUniformsSize = (instanceUniformsSize + 255) & ~255;

    // TODO: don't malloc here
    char *instanceUniforms = (char *)malloc(instanceUniformsSize * numVisibleMeshes);

    char *instanceUniformsPtr = instanceUniforms;

    for (PVMeshInstance *instance : instances)
        if (instance->visible) {
            instance->updateUniforms(flags, *(PVMeshInstanceUniforms *)instanceUniformsPtr);
            instanceUniformsPtr += instanceUniformsSize;
        }

    // TODO: static draw?
    this->lightUniforms->setData(&lightUniforms, sizeof(lightUniforms), GL_STATIC_DRAW);
    this->meshInstanceUniforms->setData(instanceUniforms, instanceUniformsSize * numVisibleMeshes, GL_STATIC_DRAW);

    drawView(flags, shader, viewMatrix, projectionMatrix, viewPosition);

    free(instanceUniforms);
}

int PVScene::getNumMeshInstances() {
    return instances.size();
}

PVMeshInstance *PVScene::getMeshInstance(int idx) {
    return instances[idx];
}

int PVScene::getNumLights() {
    return lights.size();
}

PVLight *PVScene::getLight(int idx) {
    return lights[idx];
}

int PVScene::getNumCameras() {
    return 1;
}

Camera *PVScene::getCamera(int idx) {
    return camera;
}
