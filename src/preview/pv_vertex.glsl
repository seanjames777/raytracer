layout(location=0) in vec3 in_position;
#if SUPPORT_LIGHTING
layout(location=1) in vec3 in_normal;
layout(location=2) in vec3 in_tangent;
layout(location=3) in vec2 in_uv;
#endif

layout(std140) uniform CameraUniforms {
    mat4 viewProjection;
    vec3 viewPosition;
};

layout(std140) uniform LightUniforms {
    vec3 lightPositions[4];
    vec3 lightColors[4];
    int  numLights;
    bool enableLighting;
};

layout(std140) uniform MeshInstanceUniforms {
    mat4  transform;
    mat3  orientation;
    vec3  diffuseColor;
    vec3  specularColor;
    float _pad0; // TODO
    float specularPower;
    float normalMapScale;
    bool  hasDiffuseTexture;
    bool  hasNormalTexture;
};

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;

#if SUPPORT_LIGHTING
out vec3 var_position;
out vec3 var_normal;
out vec3 var_tangent;
out vec2 var_uv;
out vec3 var_viewDirection;
#endif

void main() {
    vec4 position = transform * vec4(in_position, 1);

    gl_Position = viewProjection * position;
    
#if SUPPORT_LIGHTING
    var_position = position.xyz;
    var_normal = orientation * in_normal;
    var_tangent = orientation * in_tangent;
    var_uv = in_uv;
    var_viewDirection = position.xyz - viewPosition; // TODO: could just normalize in FS
#endif
}
