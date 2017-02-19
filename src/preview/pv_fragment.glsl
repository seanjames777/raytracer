#if SUPPORT_LIGHTING
in vec3 var_position;
in vec3 var_normal;
in vec3 var_tangent;
in vec2 var_uv;
in vec3 var_viewDirection;

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

out vec4 out_color;

void main() {
    vec3 normal = normalize(var_normal);
    vec3 tangent = normalize(var_tangent);
    vec3 bitangent = cross(tangent, normal);
    vec3 viewDir = normalize(var_viewDirection);

    if (hasNormalTexture) {
        vec3 tbn = texture(normalTexture, var_uv).rgb * 2.0 - 1.0;
        normal = normalize(tbn.x * normalMapScale * tangent + tbn.y * normalMapScale * bitangent + tbn.z * normal);
    }

    vec3 lighting = vec3(0, 0, 0);

    vec3 diffuse = diffuseColor;

    if (hasDiffuseTexture) diffuse = texture(diffuseTexture, var_uv).rgb;

    if (enableLighting) {
        for (int i = 0; i < numLights; i++) {
            vec3 l = lightPositions[i] - var_position;
            float r = length(l);

            l /= r;
            r = max(r, 0.001);

            float ndotl = clamp(dot(normal, l), 0, 1) / (r * r);

            vec3 reflDir = reflect(l, normal);
            float rdotv = pow(clamp(dot(reflDir, viewDir), 0, 1), specularPower);

            lighting += ndotl * (diffuse + specularColor * rdotv) * lightColors[i];
        }
    }
    else {
        float ndotl = clamp(dot(normal, -viewDir), 0.2, 1.0);
        lighting = ndotl * diffuse;
    }

    out_color = vec4(lighting, 1);
}
#else
void main() {
}
#endif
