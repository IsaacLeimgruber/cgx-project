#version 410 core
in vec3 vpoint;
out vec2 uv;


void main() {
    gl_Position = vec4(vpoint, 1.0);
    uv = (vpoint.xy + vec2(1.0, 1.0)) * 0.5f;
}
