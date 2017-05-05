#version 410 core
in vec2 uv;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
out vec4 color;

void main() {

    color = texture(colorTex, uv);

}
