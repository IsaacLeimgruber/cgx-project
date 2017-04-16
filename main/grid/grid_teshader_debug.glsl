#version 410 core

layout(quads, equal_spacing, ccw) in;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 normalMatrix;
uniform vec3 lightPos;
uniform vec2 zoomOffset;
uniform float zoom;

uniform sampler2D heightMap;
uniform sampler2D normalMap;

in vec3 vpoint_TE[];
in vec2 uv_TE[];

out vec4 vpoint_MV_G;
out vec4 vpoint_M_G;
out vec4 normal_MV_G;
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
    mat4 VP = projection * view;

    // Interpolate the attributes of the output vertex using the barycentric coordinates
    uv_G = interpolate2D(uv_TE[0], uv_TE[1], uv_TE[2], uv_TE[3]);
    vec3 vpoint_G = interpolate3D(vpoint_TE[0], vpoint_TE[1], vpoint_TE[2], vpoint_TE[3]);

    // Set height for generated (and original) vertices
    vheight_G = 1.3 * pow(texture(heightMap, (uv_G+zoomOffset) * zoom).r, 3);
    vpoint_G.y = vheight_G - 0.1f;
    vpoint_M_G  = model * vec4(vpoint_G, 1.0);
    vpoint_MV_G = view * vpoint_M_G;
    gl_Position = view * model * vec4(vpoint_G,1.0);


    vec3 n = (texture(normalMap, (uv_G+zoomOffset) * zoom).xyz * 2.0) - 1.0f;
    normal_MV_G = normalMatrix * vec4(n * 0.1, 1.0);
    //Lighting
    lightDir_G = normalize((view * vec4(lightPos,1.0) - vpoint_MV_G).xyz);
    viewDir_G = -normalize(vpoint_MV_G.xyz);
}
