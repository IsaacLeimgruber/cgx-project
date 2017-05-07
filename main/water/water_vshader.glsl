#version 410 core

uniform sampler2D heightMap;
uniform vec2 translation;

in vec2 gridPos;

out float terrainHeight_TC;
out vec2 uv_TC;
out vec2 terrainGradient_TC;
out vec3 vpoint_TC;

const float waterHeight = 0.0f;

void main() {
    //Outputs UV coordinate
    uv_TC = (gridPos + vec2(1.0f, 1.0f)) * 0.5f;

    vec3 terrainHDxDy = texture(heightMap, uv_TC).xyz;
    terrainGradient_TC = normalize(vec2(terrainHDxDy.y, -terrainHDxDy.z));
    terrainHeight_TC = terrainHDxDy.x;

    vpoint_TC = vec3(gridPos.x + translation.x, waterHeight, -gridPos.y - translation.y);

}
