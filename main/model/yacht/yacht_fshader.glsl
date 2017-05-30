#version 410 core

in vec2 TexCoords;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

uniform sampler2D texture_diffuse1;
uniform sampler2DShadow shadowMap;
uniform bool use_tex;
uniform bool mirror_pass;
uniform vec3 diffuse_color;
uniform vec3 specular_color;
uniform vec3 light_dir;
uniform mat4 NORMALM;
uniform vec3 viewPos;
uniform vec3 La, Ld, Ls;
uniform float alpha;
uniform float max_vpoint_World_F;
uniform float threshold_vpoint_World_F;

uniform float time;
uniform vec2 offset;

in vec2 uv_F;
in vec3 normal_MV_F;
in vec3 vpoint_MV_F;
in vec3 vpoint_F;
in vec3 viewDir_MV_F;
in vec4 shadowCoord_F;
in vec2 vpoint_World_F;

const int numSamplingPositions = 9;
const vec3 brightnessTreshold = vec3(1.0, 1.0, 1.0);

uniform vec2 kernel[9] = vec2[]
(
   vec2(0.95581f, -0.18159f), vec2(0.50147f, -0.35807f), vec2(0.69607f, 0.35559f),
   vec2(-0.0036825f, -0.59150f), vec2(0.15930f, 0.089750f), vec2(-0.65031f, 0.058189f),
   vec2(0.11915f, 0.78449f), vec2(-0.34296f, 0.51575f), vec2(-0.60380f, -0.41527f)
);

// generates pseudorandom number in [0, 1]
// seed - world space position of a fragemnt
// freq - modifier for seed. The bigger, the faster
// the pseudorandom numbers will change with change of world space position
float random(in vec3 seed, in float freq)
{
   // project seed on random constant vector
   float dt = dot(floor(seed * freq), vec3(53.1215f, 21.1352f, 9.1322f));
   // return only fractional part
   return fract(sin(dt) * 2105.2354f);
}

// returns random angle
float randomAngle(in vec3 seed, in float freq)
{
   return random(seed, freq) * 6.283285f;
}

void main()
{
    if (mirror_pass && vpoint_F.y < -0.005f) {
        discard;
    }

    vec3 lightDir = normalize((NORMALM * vec4(light_dir, 1.0)).xyz);

    vec3 viewDir = normalize(viewDir_MV_F);
    vec3 normal = normalize(normal_MV_F);
    float cosNL = dot(normal, lightDir);

    vec3 texCol = texture(texture_diffuse1, uv_F).rgb;
    vec3 diffuse_component = diffuse_color;

    if (use_tex){
        diffuse_component = texCol;
    }

    // -------------SHADOW MAPS----------------------//

    float bias = max(0.05f * (1.0f - cosNL), 0.005f);
    float visibility = 1.0;


    // --------------- SHADING ----------------- //
    vec3 lightingResult = diffuse_component * La;

    if(cosNL > 0.0){

        vec3 cosNLDiffused = cosNL * Ld;

        vec3 reflectionDir = normalize(2.0f * normal * cosNL - lightDir);
        lightingResult += visibility *
               ((diffuse_component * cosNLDiffused)
               +
               (specular_color * pow(max(0.0, dot(reflectionDir, viewDir)), 256.0) * Ls));
    }

    color = vec4(lightingResult, 1.0);
    color.a *= 1- smoothstep(threshold_vpoint_World_F, max_vpoint_World_F,
                              max(abs(vpoint_World_F.x), abs(vpoint_World_F.y))
                              );

    float brightness = dot(color.rgb, brightnessTreshold);

    brightColor = mix(vec4(0.0, 0.0, 0.0, 1.0), vec4(color), smoothstep(1.5, 6.0, brightness));
    brightColor.a = color.a;
}
