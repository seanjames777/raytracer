#version 410 core

layout(location=0) in vec2 in_position;
layout(location=1) in vec2 in_uv;
layout(location=2) in vec4 in_color;

uniform vec2 viewSize;

out vec2 var_uv;
out vec4 var_color;

void main() {
    vec2 position = in_position / viewSize;
    position.y = 1 - position.y;
    position = position * 2 - 1;
    gl_Position = vec4(position, 0, 1);

    var_uv = in_uv;
    var_color = in_color;
}
