#version 330
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec3 light_pos;
uniform vec2 zoomOffset;
uniform float zoom;
uniform sampler2D heightMap;

in vec2 gridPos;

//out float vheight;

out vec2 uv_TC;
//out vec3 light_dir;
//out vec3 view_dir;
out vec3 vpoint_TC;

void main() {
    //mat4 MV = view * model;
    //mat4 MVP = projection * MV;

    //Outputs UV coordinate for fragment shader. Grid coordinates are in [-1, 1] x [-1, 1]
    uv_TC = (gridPos + vec2(1.0, 1.0)) * 0.5;

    //vheight = 1.3 * pow(texture(heightMap, (uv+zoomOffset) * zoom).r, 3);

    vpoint_TC = vec3(gridPos.x, 0.0, -gridPos.y);

    //Outputs projected coordinates of the point
    //gl_Position = MVP*vec4(pos_3d, 1.f);

    //Lighting
    // 1) compute the light direction light_dir.
    //light_dir = normalize(light_pos - vpoint_mv.xyz);
    // 2) compute the view direction view_dir.
    //view_dir = -normalize(vpoint_mv.xyz);
}
