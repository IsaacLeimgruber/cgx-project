#version 410 core

in vec2 TexCoords;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

uniform sampler2D texture_diffuse1;
uniform vec3 diffuse_color;

uniform mat4 NORMALM;
uniform vec3 viewPos;
uniform vec3 La, Ld, Ls;
uniform float alpha;
uniform float max_vpoint_World_F;
uniform float threshold_vpoint_World_F;

uniform float time;
uniform vec2 offset;

in vec3 normal_MV_F;
in vec3 vpoint_MV_F;
in vec3 lightDir_MV_F;
in vec3 viewDir_MV_F;
in vec4 shadowCoord_F;

void main()
{
    vec3 lightDir = normalize(lightDir_MV_F);
    vec3 viewDir = normalize(viewDir_MV_F);
    vec3 normal = normalize(normal_MV_F);

    float cosNL = dot(normal, lightDir);

    vec3 lightingResult = diffuse_color * La;

    if(cosNL > 0.0){

        vec3 cosNLDiffused = cosNL * Ld;

        vec3 reflectionDir = normalize(2.0f * normal * cosNL - lightDir);
        lightingResult += 1.0 *
               ((diffuse_color * reflection * cosNLDiffused)
               +
               (vec3(1.0f, 1.0f, 1.0f) * pow(max(0.0, dot(reflectionDir, viewDir)), 128.0) * Ls));
    }

    color = vec4(lightingResult, 1.0);
    brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
