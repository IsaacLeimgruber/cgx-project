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
    float height = pow(texture(heightMap, (uv_TC+zoomOffset) * zoom).r, 3);
    float heightE = pow(texture(heightMap, (uv_TC+zoomOffset+vec2(5.0f * hSpacing, 0.0)) * zoom).r, 3);
    float heightN = pow(texture(heightMap, (uv_TC+zoomOffset+vec2(0.0, 5.0f * vSpacing)) * zoom).r, 3);

    terrainGradient_TC = normalize(vec2(heightE - height, -heightN + height));
    terrainHeight_TC = height - 0.1;


    vpoint_TC = vec3(gridPos.x, waterHeight, -gridPos.y);

}
