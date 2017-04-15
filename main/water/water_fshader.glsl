#version 410 core
uniform sampler2D heightMap;
uniform sampler2D mirrorTex;
uniform sampler2D normalMap;
uniform float time;
uniform vec2 offset;
in vec2 uv_F;
in vec2 reflectOffset_F;
in vec3 normal_F;
in vec3 vpoint_F;
in vec4 gl_FragCoord;

out vec4 color;

void main() {

    vec2 window_size = textureSize(mirrorTex, 0);
    float _u = gl_FragCoord.x / window_size.x;
    float _v = gl_FragCoord.y / window_size.y;

    color = mix(vec4(125,186,217,255) / 255.0, vec4(texture(mirrorTex, vec2(_u, _v) + reflectOffset_F).rgb, 0.5f), vec4(.3f));
    color.a = 0.8;
}
