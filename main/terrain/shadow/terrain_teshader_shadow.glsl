#version 410

layout(quads, fractional_even_spacing, ccw) in;

uniform mat4 SHADOWMVP;
uniform vec2 zoomOffset;
uniform float zoom;

uniform sampler2D heightMap;
uniform sampler2D normalMap;

in vec3 vpoint_TE[];
in vec2 uv_TE[];

out vec4 vpoint_F;
out vec2 uv_F;
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

    // Interpolate the attributes of the output vertex using the barycentric coordinates
    uv_F = interpolate2D(uv_TE[0], uv_TE[1], uv_TE[2], uv_TE[3]);
    vpoint_F = vec4(interpolate3D(vpoint_TE[0], vpoint_TE[1], vpoint_TE[2], vpoint_TE[3]), 1.0f);

    // Set height for generated (and original) vertices
    vheight_F = 1.3 * pow(texture(heightMap, (uv_F+zoomOffset) * zoom).r, 3);
    vpoint_F.y = vheight_F - 0.1f;

    gl_Position = SHADOWMVP * vpoint_F;
}
