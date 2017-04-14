#version 410 core

layout(quads, fractional_even_spacing, ccw) in;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 normalMatrix;
uniform vec3 light_pos;
uniform vec2 zoomOffset;
uniform float zoom;

uniform sampler2D heightMap;
uniform sampler2D normalMap;

in vec3 vpoint_TE[];
in vec2 uv_TE[];

out vec4 vpoint_MV_F;
out vec4 vpoint_M_F;
out vec4 normal_MV_F;
out vec2 uv_F;
out vec3 lightDir_F;
out vec3 viewDir_F;
out float vheight_F;

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2, vec2 v3)
{
    vec2 xlerp1 = mix(v0, v1, gl_TessCoord.x);
    vec2 xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
    vec3 xlerp1 = mix(v0, v1, gl_TessCoord.x);
    vec3 xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

void main()
{
    mat4 VP = projection * view;

    // Interpolate the attributes of the output vertex using the barycentric coordinates
    uv_F = interpolate2D(uv_TE[0], uv_TE[1], uv_TE[2], uv_TE[3]);
    vec3 vpoint_F = interpolate3D(vpoint_TE[0], vpoint_TE[1], vpoint_TE[2], vpoint_TE[3]);

    // Set height for generated (and original) vertices
    vheight_F = 1.3 * pow(texture(heightMap, (uv_F+zoomOffset) * zoom).r, 3);
    vpoint_F.y = vheight_F - 0.1f;
    vpoint_M_F  = model * vec4(vpoint_F, 1.0);
    vpoint_MV_F = view * vpoint_M_F;
    gl_Position = projection * vpoint_MV_F;


    //Lighting
    lightDir_F = normalize(light_pos - vpoint_M_F.xyz);
    viewDir_F = -normalize(vpoint_MV_F.xyz);
}
