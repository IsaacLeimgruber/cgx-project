#version 410 core

layout(quads, fractional_even_spacing, ccw) in;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform mat4 SHADOWMVP;
uniform vec3 lightPos;
uniform vec2 zoomOffset;
uniform float zoom;

uniform sampler2D heightMap;
uniform sampler2D normalMap;

in vec3 vpoint_TE[];
in vec2 uv_TE[];

out vec4 vpoint_F;
out vec4 shadowCoord_F;
out vec2 uv_F;
out vec3 lightDir_F;
out vec3 viewDir_F;
out float vheight_F;

vec2 interpolate2D(in vec2 v0, in vec2 v1, in vec2 v2, in vec2 v3)
{
    vec2 xlerp1 = mix(v0, v1, gl_TessCoord.x);
    vec2 xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

vec3 interpolate3D(in vec3 v0, in vec3 v1, in vec3 v2, in vec3 v3)
{
    vec3 xlerp1 = mix(v0, v1, gl_TessCoord.x);
    vec3 xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

void main()
{

    // Interpolate the attributes of the output vertex using the barycentric coordinates
    uv_F = interpolate2D(uv_TE[0], uv_TE[1], uv_TE[2], uv_TE[3]);
    vpoint_F = vec4(interpolate3D(vpoint_TE[0], vpoint_TE[1], vpoint_TE[2], vpoint_TE[3]), 1.0f);

    // Set height for generated (and original) vertices
    vheight_F = 1.3f * pow(texture(heightMap, (uv_F+zoomOffset) * zoom).r, 3);
    vpoint_F.y = vheight_F - 0.1f;

    vec4 vpoint_MV = MV * vpoint_F;
    //Lighting
    lightDir_F = normalize((MV * vec4(lightPos, 1.0f)).xyz - vpoint_MV.xyz);
    viewDir_F = -normalize(vpoint_MV.xyz);

    gl_Position = MVP * vpoint_F;
    shadowCoord_F = SHADOWMVP * vpoint_F;
}
