#version 410 core

uniform sampler2D heightMap;
uniform vec2 zoomOffset;
uniform float zoom;

in vec2 gridPos;

out float terrainHeight_TC;
out vec2 uv_TC;
out vec2 terrainGradient_TC;
out vec3 vpoint_TC;

const float waterHeight = 0.0f;

void main() {
    //Outputs UV coordinate
    uv_TC = (gridPos + vec2(1.0f, 1.0f)) * 0.5f;

    vec2 texDims = textureSize(heightMap, 0);
    float hSpacing = 1.0 / texDims.x;
    float vSpacing = 1.0 / texDims.y;
    float height = texture(heightMap, uv_TC).r;
    float heightE = texture(heightMap, uv_TC+vec2(5.0f * hSpacing, 0.0)).r;
    float heightN = texture(heightMap, uv_TC+vec2(0.0, 5.0f * vSpacing)).r;

    terrainGradient_TC = normalize(vec2(heightE - height, -heightN + height));
    terrainHeight_TC = height;


    vpoint_TC = vec3(gridPos.x, waterHeight, -gridPos.y);

}
