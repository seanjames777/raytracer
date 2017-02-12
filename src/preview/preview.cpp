#include <preview/preview.h>
#include <util/timer.h>
#include <kdtree/kdsahbuilder.h>

extern void profiler_push(const char *name);
extern void profiler_pop();

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
#define GLCHECK(stmt) {                                                       \
    stmt;                                                                     \
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

enum PVRenderFlagsPrivate {
    PVRenderFlagPrivateDepthOnly     = (1 << 4),
    PVRenderFlagPrivatePreserveDepth = (1 << 5)
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
    // TODO: could bind a range
    GLCHECK(glBindBufferBase(target, index, buffer));
}

PVShader::PVShader(const char *vs_path, const char *fs_path, const GLchar **defines, int num_defines) {
    program = GLCHECK(glCreateProgram());

    const GLchar **sources = (const GLchar **)malloc(sizeof(GLchar *) * (num_defines + 1));
    GLint *lengths = (GLint *)malloc(sizeof(GLint) * (num_defines + 1));

    for (int i = 0; i < num_defines; i++) {
        sources[i] = defines[i];
        lengths[i] = strlen(defines[i]);
    }

    if (vs_path) {
        GLuint vs = GLCHECK(glCreateShader(GL_VERTEX_SHADER));

        FILE *fd = fopen(relToExeDir(vs_path).c_str(), "r");

        fseek(fd, 0, SEEK_END);
        int vs_len = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        char *vs_source = (char *)malloc(vs_len);
        fread(vs_source, 1, vs_len, fd);

        fclose(fd);

        sources[num_defines] = vs_source;
        lengths[num_defines] = vs_len;

        GLCHECK(glShaderSource(vs, num_defines + 1, sources, lengths));

        free(vs_source);

        GLCHECK(glCompileShader(vs));

        GLint result = GL_FALSE;
        int length;

        GLCHECK(glGetShaderiv(vs, GL_COMPILE_STATUS, &result));
        GLCHECK(glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &length));
        if (length > 0){
            std::vector<char> message(length+1);
            GLCHECK(glGetShaderInfoLog(vs, length, NULL, &message[0]));
            printf("%s\n", &message[0]);
        }

        GLCHECK(glAttachShader(program, vs));

        GLCHECK(glDeleteShader(vs));
    }

    if (fs_path) {
        GLuint fs = GLCHECK(glCreateShader(GL_FRAGMENT_SHADER));

        FILE *fd = fopen(relToExeDir(fs_path).c_str(), "r");

        fseek(fd, 0, SEEK_END);
        int fs_len = ftell(fd);
        fseek(fd, 0, SEEK_SET);

        char *fs_source = (char *)malloc(fs_len);
        fread(fs_source, 1, fs_len, fd);

        fclose(fd);

        sources[num_defines] = fs_source;
        lengths[num_defines] = fs_len;

        GLCHECK(glShaderSource(fs, num_defines + 1, sources, lengths));

        free(fs_source);

        GLCHECK(glCompileShader(fs));

        GLint result = GL_FALSE;
        int length;

        GLCHECK(glGetShaderiv(fs, GL_COMPILE_STATUS, &result));
        GLCHECK(glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &length));
        if (length > 0){
            std::vector<char> message(length+1);
            GLCHECK(glGetShaderInfoLog(fs, length, NULL, &message[0]));
            printf("%s\n", &message[0]);
        }

        GLCHECK(glAttachShader(program, fs));

        GLCHECK(glDeleteShader(fs));
    }

    free(sources);
    free(lengths);

    GLCHECK(glLinkProgram(program));

    GLint result = GL_FALSE;
    int length;

    GLCHECK(glGetProgramiv(program, GL_LINK_STATUS, &result));
    GLCHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length));
    if (length > 0){
        std::vector<char> message(length+1);
        GLCHECK(glGetProgramInfoLog(program, length, NULL, &message[0]));
        printf("%s\n", &message[0]);
    }
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

PVFramebuffer::PVFramebuffer() {
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

PVMesh::PVMesh(
    util::vector<Triangle, 16> & triangles,
    bool reverseWinding,
    const std::string & name)
    : vertexCount(triangles.size() * 3),
      name(name)
{
    KDSAHBuilder builder(kdTree, triangles, 12.0f, 1.0f);
    builder.build();

    std::vector<PVVertex> vertices;

    for (int j = 0; j < triangles.size(); j++) {
        const Triangle & tri = triangles[j];

        for (int k = 0; k < 3; k++) {
            const Vertex & v = tri.v[reverseWinding ? 2 - k : k];

            // TODO: preserve/generate index buffers

            PVVertex pv;
            for (int l = 0; l < 3; l++) {
                pv.position[l] = v.position[l];
                pv.normal[l] = (reverseWinding ? -1.0f : 1.0f) *  v.normal[l];
                pv.tangent[l] = v.tangent[l];
            }
            for (int l = 0; l < 2; l++) {
                pv.uv[l] = v.uv[l];
            }

            vertices.push_back(pv);
        }
    }

    // TODO: preserve state
    GLCHECK(glGenVertexArrays(1, &vao));
    GLCHECK(glBindVertexArray(vao));

    vertexBuffer = new PVBuffer();
    vertexBuffer->bind(GL_ARRAY_BUFFER);

    vertexBuffer->setData(&vertices[0], sizeof(PVVertex) * vertexCount, GL_STATIC_DRAW);

    GLCHECK(glEnableVertexAttribArray(0));
    GLCHECK(glEnableVertexAttribArray(1));
    GLCHECK(glEnableVertexAttribArray(2));
    GLCHECK(glEnableVertexAttribArray(3));

    // TODO: aligning to 4 bytes may actually help...
    GLCHECK(glVertexAttribPointer(0, 3, GL_FLOAT, false, 44, (void *)0));
    GLCHECK(glVertexAttribPointer(1, 3, GL_FLOAT, false, 44, (void *)12));
    GLCHECK(glVertexAttribPointer(2, 3, GL_FLOAT, false, 44, (void *)24));
    GLCHECK(glVertexAttribPointer(3, 2, GL_FLOAT, false, 44, (void *)36));

    GLCHECK(glBindVertexArray(0));
}

PVMesh::~PVMesh() {
    // TODO: delete stuff
    GLCHECK(glDeleteVertexArrays(1, &vao));
}

void PVMesh::draw() {
    GLCHECK(glBindVertexArray(vao));
    GLCHECK(glDrawArrays(GL_TRIANGLES, 0, vertexCount));
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
    PVTexture *diffuseTexture,
    PVTexture *normalTexture,
    float3 position,
    float3 rotation,
    float3 scale,
    const std::string & name)
    : mesh(mesh),
      diffuseColor(diffuseColor),
      specularColor(specularColor),
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
    // TODO: make nicer functions
    return ::translation(position.x, position.y, position.z) *
           ::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
           ::scale(scale.x, scale.y, scale.z);
}

void PVMeshInstance::draw(PVRenderFlags flags, PVShader *shader) {
    GLuint diffuseTextureLocation = shader->getUniformLocation("diffuseTexture");
    GLuint normalTextureLocation = shader->getUniformLocation("normalTexture");
    GLuint hasDiffuseTextureLocation = shader->getUniformLocation("hasDiffuseTexture");
    GLuint hasNormalTextureLocation = shader->getUniformLocation("hasNormalTexture");
    GLuint diffuseColorLocation = shader->getUniformLocation("diffuseColor");
    GLuint specularColorLocation = shader->getUniformLocation("specularColor");
    GLuint specularPowerLocation = shader->getUniformLocation("specularPower");
    GLuint transformLocation = shader->getUniformLocation("transform");
    GLuint orientationLocation = shader->getUniformLocation("orientation");

    GLCHECK(glUniform3f(diffuseColorLocation, diffuseColor.x, diffuseColor.y, diffuseColor.z));
    GLCHECK(glUniform3f(specularColorLocation, specularColor.x, specularColor.y, specularColor.z));
    GLCHECK(glUniform1f(specularPowerLocation, specularPower));

    float4x4 transform = getTransform();
    // TODO: doubly transposed
    float3x3 orientation = upper3x3(transpose(inverse(transform)));
    
    GLCHECK(glUniformMatrix4fv(transformLocation, 1, true, &transform.m[0][0]));        
    GLCHECK(glUniformMatrix3fv(orientationLocation, 1, true, &orientation.m[0][0]));        

    GLCHECK(glUniform1i(diffuseTextureLocation, 0));
    GLCHECK(glUniform1i(normalTextureLocation, 1));

    GLCHECK(glUniform1i(hasDiffuseTextureLocation, (flags & PVRenderFlagEnableTexturing) != 0 && diffuseTexture != nullptr));
    GLCHECK(glUniform1i(hasNormalTextureLocation, (flags & PVRenderFlagEnableNormalMapping) != 0 && normalTexture != nullptr));

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
            submesh->getTriangles(),
            instance->reverseWinding,
            submesh->getName());

        PVMeshInstance *pvinstance = new PVMeshInstance(
            pvmesh,
            material->diffuseColor,
            float3(0.8f),
            16.0f,
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

    GLCHECK(glBindVertexArray(0));

    meshSampler = new PVSampler(PVFilterLinear, PVFilterLinear, PVFilterNearest, PVAddressRepeat, PVAddressRepeat, 1);
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

void PVScene::drawPass(PVRenderFlags flags,
                       PVShader *shader,
                       float4x4 viewMatrix,
                       float4x4 projectionMatrix,
                       float3 viewPosition,
                       PVFramebuffer *framebuffer)
{
    int width = 0;
    int height = 0;
    int sampleCount = 0;

    if (PVTexture *depthAttachment = framebuffer->getDepthAttachment()) {
        width = depthAttachment->getWidth();
        height = depthAttachment->getHeight();
        sampleCount = depthAttachment->getSampleCount();
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (PVTexture *colorAttachment = framebuffer->getColorAttachment(i)) {
                width = colorAttachment->getWidth();
                height = colorAttachment->getHeight();
                sampleCount = colorAttachment->getSampleCount();
                break;
            }
        }
    }

    if (flags & PVRenderFlagPrivateDepthOnly) {
        GLCHECK(glColorMask(false, false, false, false));
        //GLCHECK(glDrawBuffer(GL_NONE)); // TODO
    }

    shader->bind();

    // TODO: Don't fill out all these uniforms for depth only passes

    float4x4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    GLuint viewProjectionLocation = shader->getUniformLocation("viewProjection");
    GLCHECK(glUniformMatrix4fv(viewProjectionLocation, 1, true, &viewProjectionMatrix.m[0][0]));

    GLuint viewPositionLocation = shader->getUniformLocation("viewPosition");
    GLCHECK(glUniform3f(viewPositionLocation, viewPosition.x, viewPosition.y, viewPosition.z));

    framebuffer->bind(GL_FRAMEBUFFER);

    // TODO: don't need to clear color without color attachments
    GLCHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
    GLCHECK(glClearDepth(1.0f));

    GLCHECK(glClear(GL_COLOR_BUFFER_BIT | ((flags & PVRenderFlagPrivatePreserveDepth ? 0 : GL_DEPTH_BUFFER_BIT))));

    if (sampleCount > 0)
        GLCHECK(glEnable(GL_MULTISAMPLE));

    GLCHECK(glEnable(GL_DEPTH_TEST));

    GLCHECK(glDepthFunc((flags & PVRenderFlagPrivatePreserveDepth) ? GL_EQUAL : GL_LESS));

    if (flags & PVRenderFlagEnableCulling)
        GLCHECK(glEnable(GL_CULL_FACE));

    GLCHECK(glCullFace(GL_BACK));
    GLCHECK(glFrontFace(GL_CW));

    GLCHECK(glViewport(0, 0, width, height));

    for (int i = 0; i < 2; i++)
        meshSampler->bind(i);

    for (PVMeshInstance *instance : instances)
        if (instance->visible)
            instance->draw(flags, shader);

    //GLCHECK(glDrawBuffer(GL_BACK));
    GLCHECK(glColorMask(true, true, true, true));

    GLCHECK(glDisable(GL_CULL_FACE));

    GLCHECK(glDepthFunc(GL_LESS));

    GLCHECK(glDisable(GL_DEPTH_TEST));

    if (sampleCount > 0)
        GLCHECK(glDisable(GL_MULTISAMPLE));

    GLCHECK(glUseProgram(0));

    GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void PVScene::draw(PVRenderFlags flags,
                   int maxLights,
                   PVFilterMode filterMode,
                   int anisotropy,
                   float4x4 viewMatrix,
                   float4x4 projectionMatrix,
                   float3 viewPosition,
                   PVFramebuffer *framebuffer)
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

    shader->bind();

    GLuint lightColorLocation = shader->getUniformLocation("lightColor");
    GLuint lightPositionLocation = shader->getUniformLocation("lightPosition");
    GLuint numLightsLocation = shader->getUniformLocation("numLights");
    GLuint enableLightingLocation = shader->getUniformLocation("enableLighting");

    float lightPositions[3 * 4];
    float lightColors[3 * 4];

    int numLights = lights.size();

    if (numLights > maxLights)
        numLights = maxLights;

    for (int i = 0; i < numLights; i++) {
        PVLight *light = lights[i];

        float3 lightPosition = light->getPosition();
        float3 lightColor = light->getColor() * light->getIntensity();

        lightPositions[i * 3 + 0] = lightPosition.x;
        lightPositions[i * 3 + 1] = lightPosition.y;
        lightPositions[i * 3 + 2] = lightPosition.z;

        lightColors[i * 3 + 0] = lightColor.x;
        lightColors[i * 3 + 1] = lightColor.y;
        lightColors[i * 3 + 2] = lightColor.z;
    }

    GLCHECK(glUniform3fv(lightPositionLocation, numLights, lightPositions));
    GLCHECK(glUniform3fv(lightColorLocation, numLights, lightColors));
    GLCHECK(glUniform1i(numLightsLocation, numLights));
    GLCHECK(glUniform1i(enableLightingLocation, (flags & PVRenderFlagEnableLighting) != 0));

    drawPass(flags, shader, viewMatrix, projectionMatrix, viewPosition, framebuffer);
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
