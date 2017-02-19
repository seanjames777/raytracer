#version 410 core

layout(location=0) in vec3 in_position;
layout(location=1) in vec4 in_color;

uniform mat4 viewProjection;

out vec4 var_color;

void main() {
    gl_Position = viewProjection * vec4(in_position, 1);

    var_color = in_color;
}
