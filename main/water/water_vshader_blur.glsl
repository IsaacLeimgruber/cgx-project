#version 410 core

in vec2 gridPos;

out vec2 uv_F;


void main() {
    //Outputs UV coordinate
    uv_F = (gridPos + vec2(1.0f, 1.0f)) * 0.5f;

    gl_Position = vec4(gridPos.x, -gridPos.y, 0.0f, 1.0f);

}
