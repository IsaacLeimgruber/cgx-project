#version 410 core

uniform mat4 MVP;

in vec3 vpoint;

out vec2 uv;

void main() {
    uv = (vpoint.xz + vec2(1.0f, 1.0f)) * 0.5f;
    gl_Position = MVP * vec4(vpoint, 1.0);
}
