#version 410 core

in vec2 gridPos;

out vec2 uv_TC;
out vec3 vpoint_TC;

const float waterHeight = 0.0f;

void main() {

    //Outputs UV coordinate
    uv_TC = (gridPos + vec2(1.0f, 1.0f)) * 0.5f;

    vpoint_TC = vec3(gridPos.x, waterHeight, -gridPos.y);

}
