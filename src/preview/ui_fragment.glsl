#version 410 core

in vec2 var_uv;
in vec4 var_color;

uniform sampler2D tex;

out vec4 out_color;

void main() {
    out_color = texture(tex, var_uv) * var_color;
}
