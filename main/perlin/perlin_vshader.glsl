#version 330 core
in vec3 vpoint;
in vec2 vtexcoord;
out vec2 uv;
uniform vec3 translation;
uniform float xscale;
uniform float yscale;

void main() {
  gl_Position = vec4(vec3(vpoint.x * xscale, vpoint.y * yscale, vpoint.z) + translation, 1.0);
  uv = vtexcoord;
}
