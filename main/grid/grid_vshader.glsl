#version 330
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

uniform vec3 light_pos;

//TODO: remove this once we have heightMap generatoin implemented
uniform float time;

uniform sampler2D heightMap;

in vec2 gridPos;

out vec2 uv;
out vec3 light_dir;
out vec3 view_dir;

//TODO: choose between flat and phong shading
out vec3 mv_normal;
out vec4 vpoint_mv;

void main() {
    mat4 MV = view * model;
    mat4 MVP = projection * MV;

    //Outputs UV coordinate for fragment shader. Grid coordinates are in [-1, 1] x [-1, 1]
    uv = (gridPos + vec2(1.0, 1.0)) * 0.5;

    //float height = 1.0/2550 * texture(heightMap, uv);
    float height = 0.1*sin(2 * 6.4 * (uv.x + uv.y) + time);
    vec3 vnormal = vec3(0.0);
    vec3 pos_3d = vec3(gridPos.x, height, -gridPos.y);


    //TODO:If Phong shading then compute vertex normal based on heightmap and 2D pos of the point of the grid
    /*  Shape of grid:
     *      x-----x
     *      |   / |
     *      |  /  |
     *      | /   |
     *      x-----x
     *  Bottom-left corner is (i,j) and top right is (i+1,j+1)
     */

    //Otherwise forwards point to fragment shader so it can compute surface normal:
    vpoint_mv = MV * vec4(pos_3d, 1.0f);

    //Outputs projected coordinates of the point
    gl_Position = MVP*vec4(pos_3d, 1.f);
    //Outputs rectified surface normal
    mv_normal = normalize((inverse(transpose(MV)) * vec4(vnormal, 1.0)).xyz);

    //Lighting
    // 1) compute the light direction light_dir.
    light_dir = normalize(light_pos - vpoint_mv.xyz);
    // 2) compute the view direction view_dir.
    view_dir = -normalize(vpoint_mv.xyz);
}
