#version 410 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform mat4 SHADOWMVP;
uniform vec3 lightPos;

out vec2 TexCoords;
out vec3 normal_MV_F;
out vec3 lightDir_MV_F;
out vec3 viewDir_MV_F;
out vec3 vpoint_MV_F;
out vec4 shadowCoord_F;


void main()
{

    vec3 vpoint_MV = MV * position;
    normal_MV_F = normalize((NORMALM * vec4(normal, 1.0f)).xyz);
    lightDir_MV_F = normalize((MV * vec4(lightPos, 1.0f)).xyz - vpoint_MV.xyz);
    viewDir_MV_F = -normalize(vpoint_MV.xyz);
    vpoint_MV_F = vpoint_MV.xyz;

    gl_Position = MVP * vec4(position, 1.0f);
    shadowCoord_F = SHADOWMVP * vec4(vpoint_F, 1.0f);

    TexCoords = texCoords;
}
