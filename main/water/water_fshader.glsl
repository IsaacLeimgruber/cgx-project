#version 410 core
uniform sampler2D diffuseMap;
uniform sampler2D heightMap;
uniform sampler2D mirrorMap;
uniform sampler2D normalMap;
uniform sampler2DShadow shadowMap;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform vec3 viewPos;
uniform vec3 light_dir;
uniform vec3 La, Ld, Ls;
uniform float alpha;
uniform float max_vpoint_World_F;
uniform float threshold_vpoint_World_F;

uniform float time;
uniform vec2 offset;

in float tHeight_F;
in vec2 uv_F;
in vec3 normal_F;
in vec3 normal_MV_F;
in vec3 vpoint_F;
in vec3 vpoint_MV_F;
in vec3 lightDir_F;
in vec3 viewDir_MV_F;
in vec4 gl_FragCoord;
in vec4 shadowCoord_F;
in vec2 vpoint_World_F;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

const vec3 WATER_COLOR = vec3(75.0f,126.0f,157.0f) / 255.0f;
const vec3 brightnessTreshold = vec3(1.0, 1.0, 1.0);
const vec3 Y = vec3(0.0f, 1.0f, 0.0f);
const float cosWaterReflectionAngleStart = 0.20f;
const float cosWaterReflectionAngleEnd = 0.80f;
const float waterReflectionDistanceStart = 2.0f;
const float waterReflectionDistanceEnd = 10.0f;
const float rippleNormalWeight = 0.15f;
const float scumScale = 2.0f;

const int numSamplingPositions = 9;
uniform vec2 kernel[9] = vec2[9]
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

//Assume dest is opaque
vec4 blendColors(in vec4 src, in vec3 dst){
    vec4 v;

    v.a = 1.0f;
    v.rgb = (src.rgb * src.a + dst.rgb * (1.0 - src.a));

    return v;
}

vec4 blendColors(in vec4 src, in vec4 dst){
    vec4 v;

    v.a = src.a + dst.a * (1.0f - src.a);
    v.rgb = (src.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a)) / v.a;

    return v;
}

void main() {
    vec2 window_size = textureSize(mirrorMap, 0);
    vec3 lightDir = normalize((NORMALM * vec4(light_dir, 1.0)).xyz);
    vec3 viewDir = normalize(viewDir_MV_F);
    vec3 normal = normalize(normal_F);
    float _u = gl_FragCoord.x / window_size.x;
    float _v = 1.0f - gl_FragCoord.y / window_size.y;
    float valTimeShift = 0.01 * time;
    float visibility = 0.0f;

    vec3 rippleNormal =
            texture(normalMap, (uv_F + vec2(0.0, valTimeShift)) * 3.0).rgb * 2.0 - 1.0f
            +
            texture(normalMap, (uv_F + vec2(0.0, -valTimeShift)) * 3.0).rgb * 2.0 - 1.0f;

    rippleNormal = vec3(rippleNormal.x, rippleNormal.z, -rippleNormal.y);
    vec3 completeNormal = normalize(normal + rippleNormalWeight * rippleNormal);

    vec3 normal_MV = normalize((NORMALM * vec4(completeNormal, 1.0f)).xyz);
    float cosNL = dot(normal_MV, lightDir);
    float bias = max(0.05f * (1.0f - cosNL), 0.005f);

    float fadingValue = smoothstep(threshold_vpoint_World_F, max_vpoint_World_F,
                                  max(abs(vpoint_World_F.x), abs(vpoint_World_F.y))
                                  );

    // generate random rotation angle for each fragment
    float angle = randomAngle(gl_FragCoord.xyz, 15.0f);
    float s = sin(angle);
    float c = cos(angle);
    float PCFRadius = 1/1000.0f;
    for(int i=0; i < numSamplingPositions; i++)
    {
      // rotate offset
      vec2 rotatedOffset = vec2(kernel[i].x * c + kernel[i].y * -s, kernel[i].x * s + kernel[i].y * c);
      vec3 samplingPos = shadowCoord_F.xyz;
      samplingPos += vec3(rotatedOffset * PCFRadius, -bias);
      visibility += texture(shadowMap, samplingPos / shadowCoord_F.w);
    }
    visibility /= numSamplingPositions;
    visibility *= 1 - fadingValue;

    //Flat normal is the projection of the wave normal onto the mirror surface
    vec3 flatNormal = completeNormal - dot(completeNormal, Y) * Y;
    //Compute how the flat normal look in camera space
    vec3 eyeNormal = (NORMALM * vec4(flatNormal, 1.0f)).xyz;
    //Compute distortion
    vec2 reflectOffset = normalize(eyeNormal.xy) * length (flatNormal) * rippleNormalWeight;

    vec3 reflection = texture(mirrorMap, vec2(_u, _v) + reflectOffset).rgb;


    vec4 scumColor = texture(diffuseMap, (uv_F + vec2(0.0f, valTimeShift)) * scumScale).rgba;
    vec3 lightingResult = reflection * La;
    vec3 lightingResultScum =  scumColor.rgb * 2.0 * La;

    if(cosNL > 0.0){

        vec3 cosNLDiffused = cosNL * Ld;

        vec3 reflectionDir = normalize(2.0f * normal_MV * cosNL - lightDir);
        lightingResult += visibility *
               ((vec3(0.8, 0.8, 0.8) * reflection * cosNLDiffused)
               +
               (vec3(1.0f, 1.0f, 1.0f) * pow(max(0.0, dot(reflectionDir, viewDir)), 512.0) * Ls));
        lightingResultScum += visibility *
               (lightingResultScum * 2.0 * cosNLDiffused);
    }

    float reflectionAlpha = mix(0.95f, 0.3f, min(
                                smoothstep(cosWaterReflectionAngleStart, cosWaterReflectionAngleEnd, dot(viewDir, normal_MV))
                                ,
                                1.0 - smoothstep(waterReflectionDistanceStart, waterReflectionDistanceEnd, -vpoint_MV_F.z))
                                );

    vec4 seaColor = vec4(lightingResult, reflectionAlpha);
    vec4 tmpColor = blendColors(vec4(lightingResultScum, scumColor.a), seaColor);
    color = mix(seaColor, tmpColor, smoothstep(-0.15, 0.015, tHeight_F) * smoothstep(0.001, 0.006, vpoint_F.y));

    // compute tranparency factor for a fog-effect
    color.a *= 1 - fadingValue;

    float brightness = dot(color.rgb, brightnessTreshold);

    brightColor = mix(vec4(0.0, 0.0, 0.0, color.a), vec4(color), smoothstep(1.5, 6.0, brightness));
    brightColor.a = color.a;

}
