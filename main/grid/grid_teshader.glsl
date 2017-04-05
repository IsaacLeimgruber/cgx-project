#version 410 core

layout(triangles, equal_spacing, ccw) in;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec3 light_pos;
uniform vec2 zoomOffset;
uniform float zoom;

uniform sampler2D heightMap;

in vec3 vpoint_TE[];
in vec2 uv_TE[];

out vec4 vpoint_MV_F;
out vec2 uv_F;
out vec3 lightDir_F;
out vec3 viewDir_F;
out float vheight_F;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

void main()
{
    mat4 MV = view * model;
    mat4 MVP = projection * MV;

    // Interpolate the attributes of the output vertex using the barycentric coordinates
    uv_F = interpolate2D(uv_TE[0], uv_TE[1], uv_TE[2]);
    vec3 vpoint_F = interpolate3D(vpoint_TE[0], vpoint_TE[1], vpoint_TE[2]);

    vheight_F = 1.3 * pow(texture(heightMap, (uv_F+zoomOffset) * zoom).r, 3);

    //vpoint_F.y += vheight_F;

    gl_Position = MVP * vec4(vpoint_F, 1.0);

    vpoint_MV_F = MV * vec4(vpoint_F, 1.0);

    //Lighting
    // 1) compute the light direction light_dir.
    lightDir_F = normalize(light_pos - vpoint_MV_F.xyz);
    // 2) compute the view direction view_dir.
    viewDir_F = -normalize(vpoint_MV_F.xyz);
}
