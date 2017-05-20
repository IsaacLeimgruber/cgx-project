#version 410 core

uniform sampler2D heightMap;
uniform vec2 translation;
uniform vec2 translationToSceneCenter;

in vec2 gridPos;

out vec2 uv_TC;
out vec3 vpoint_TC;
out vec2 vpoint_World_TC;
void main() {

    //Outputs UV coordinate for fragment shader. Grid coordinates are in [-1, 1] x [-1, 1]
    uv_TC = (gridPos + vec2(1.0f, 1.0f)) * 0.5f;

    float vheight = texture(heightMap, uv_TC).r;

    //Already sets displacement so we can cull patches that fall outside the view frustrum
    vpoint_TC = vec3(gridPos.x + translation.x, vheight, -gridPos.y - translation.y);
    vpoint_World_TC = translationToSceneCenter + gridPos;
}
