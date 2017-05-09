#version 410 core
in vec2 uv;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
out vec4 color;

const float alphaThreshold = 0.1f;
void main() {

    float depthVal = texture(colorTex, uv * 10.0).x;

    depthVal = pow(depthVal, 1.0);

    float alphaComponent = depthVal >  alphaThreshold ? depthVal : 0.0f;

    color = vec4(depthVal, depthVal, depthVal, alphaComponent);
}
