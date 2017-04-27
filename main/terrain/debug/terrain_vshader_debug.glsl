#version 410 core

uniform vec2 zoomOffset;
uniform float zoom;
uniform sampler2D heightMap;

in vec2 gridPos;

out vec2 uv_TC;
out vec3 vpoint_TC;

void main() {

    //Outputs UV coordinate for fragment shader. Grid coordinates are in [-1, 1] x [-1, 1]
    uv_TC = (gridPos + vec2(1.0, 1.0)) * 0.5;

    float vheight = 1.3 * pow(texture(heightMap, (uv_TC+zoomOffset) * zoom).r, 3) -0.1f;

    //Already sets displacement so we can cull patches that fall outside the view frustrum
    vpoint_TC = vec3(gridPos.x, vheight, -gridPos.y);

}
