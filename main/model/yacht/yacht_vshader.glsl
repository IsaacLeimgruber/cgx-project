#version 410 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform mat4 SHADOWMVP;
uniform vec2 translationToSceneCenter;

out vec2 uv_F;
out vec2 TexCoords;
out vec3 normal_MV_F;
out vec3 viewDir_MV_F;
out vec3 vpoint_F;
out vec3 vpoint_MV_F;
out vec2 vpoint_World_F;
out vec4 shadowCoord_F;


void main()
{

    vec4 vpoint_MV = MV * vec4(position, 1.0);
    vpoint_MV_F = vpoint_MV.xyz;
    normal_MV_F = normalize((NORMALM * vec4(normal, 1.0f)).xyz);
    viewDir_MV_F = -normalize(vpoint_MV.xyz);

    gl_Position = MVP * vec4(position, 1.0f);
    shadowCoord_F = SHADOWMVP * vec4(position, 1.0f);

    vpoint_World_F = translationToSceneCenter + position.xz;

    uv_F = texCoords;
}
