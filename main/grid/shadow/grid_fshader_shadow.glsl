#version 410

out float color;

void main() {
    color = gl_FragCoord.z;
}
