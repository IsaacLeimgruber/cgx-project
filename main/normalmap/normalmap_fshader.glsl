#version 410 core
uniform sampler2D heightMap;
uniform float textureWidth;
uniform float textureHeight;

in vec2 uv;
out vec4 color;


void main() {
    float hSpacing = 1.0 / textureWidth;
    float vSpacing = 1.0 / textureHeight;

    float h0 = texture(heightMap, uv).r;
    float h1 = texture(heightMap, uv + vec2(hSpacing, 0.0)).r;
    float h2 = texture(heightMap, uv + vec2(0.0, vSpacing)).r;
    float h3 = texture(heightMap, uv + vec2(-hSpacing, 0.0)).r;
    float h4 = texture(heightMap, uv + vec2(0.0, -vSpacing)).r;

    vec3 p0 = vec3(uv.x, h0, -uv.y);
    vec3 p1 = vec3(uv.x + hSpacing, h1, -uv.y);
    vec3 p2 = vec3(uv.x, h2, -uv.y - vSpacing);
    vec3 p3 = vec3(uv.x - hSpacing, h3, -uv.y);
    vec3 p4 = vec3(uv.x, h4, -uv.y + vSpacing);

    vec3 v1 = p1 - p0;
    vec3 v2 = p2 - p0;
    vec3 v3 = p3 - p0;
    vec3 v4 = p4 - p0;

    vec3 n = normalize(cross(v1,v2)+
                       cross(v2,v3) +
                       cross(v3,v4) +
                       cross(v4,v1));
    color = vec4((n + 1.0) / 2.0, 1.0f);
}
