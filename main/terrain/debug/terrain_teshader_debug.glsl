#version 410 core

layout(quads, equal_spacing, ccw) in;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform vec3 lightPos;

uniform sampler2D heightMap;

in vec3 vpoint_TE[];
in vec2 uv_TE[];

out vec4 vpoint_MV_G;
out vec4 vpoint_M_G;
out vec3 normal_G;
out vec2 uv_G;
out vec3 lightDir_G;
out vec3 viewDir_G;
out float vheight_G;

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

    // Interpolate the attributes of the output vertex using the barycentric coordinates
    uv_G = interpolate2D(uv_TE[0], uv_TE[1], uv_TE[2], uv_TE[3]);
    vec3 vpoint_G = interpolate3D(vpoint_TE[0], vpoint_TE[1], vpoint_TE[2], vpoint_TE[3]);

    // Set height for generated (and original) vertices
    vec3 terrainHDxDy = texture(heightMap, uv_G).rgb;
    vheight_G = terrainHDxDy.r;
    vpoint_G.y = vheight_G;
    vpoint_MV_G = MV * vec4(vpoint_G, 1.0);
    gl_Position = vec4(vpoint_G, 1.0);

    lightDir_G = normalize(lightPos - vpoint_G.xyz);
    vec3 gridNormal = normalize(vec3(-terrainHDxDy.y, 1, +terrainHDxDy.z));
    normal_G = gridNormal;

    viewDir_G = -normalize(vpoint_MV_G.xyz);
}
