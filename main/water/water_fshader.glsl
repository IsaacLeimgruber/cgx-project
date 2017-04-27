#version 410 core
uniform sampler2D heightMap;
uniform sampler2D mirrorMap;
uniform sampler2D normalMap;
uniform sampler2DShadow shadowMap;

uniform mat4 NORMALM;
uniform vec3 viewPos;
uniform vec3 La, Ld, Ls;
uniform vec3 ka, kd, ks;
uniform float alpha;

uniform float time;
uniform vec2 offset;

in vec2 uv_F;
in vec3 normal_F;
in vec3 normal_MV_F;
in vec3 vpoint_F;
in vec3 vpoint_MV_F;
in vec3 lightDir_F;
in vec3 viewDir_MV_F;
in vec4 gl_FragCoord;
in vec4 shadowCoord_F;

out vec4 color;

const vec3 WATER_COLOR = vec3(125,186,217) / 255.0;
const vec3 Y = vec3(0.0, 1.0, 0.0);
const float cosWaterReflectionAngle = 0.2;
const float waterReflectionDistance = 1.5;

const int numSamplingPositions = 9;
uniform vec2 kernel[9] = vec2[]
(
   vec2(0.95581, -0.18159), vec2(0.50147, -0.35807), vec2(0.69607, 0.35559),
   vec2(-0.0036825, -0.59150), vec2(0.15930, 0.089750), vec2(-0.65031, 0.058189),
   vec2(0.11915, 0.78449), vec2(-0.34296, 0.51575), vec2(-0.60380, -0.41527)
);

// generates pseudorandom number in [0, 1]
// seed - world space position of a fragemnt
// freq - modifier for seed. The bigger, the faster
// the pseudorandom numbers will change with change of world space position
float random(vec3 seed, float freq)
{
   // project seed on random constant vector
   float dt = dot(floor(seed * freq), vec3(53.1215, 21.1352, 9.1322));
   // return only fractional part
   return fract(sin(dt) * 2105.2354);
}

// returns random angle
float randomAngle(vec3 seed, float freq)
{
   return random(seed, freq) * 6.283285;
}

void main() {
    vec2 window_size = textureSize(mirrorMap, 0);
    vec3 lightDir = normalize(lightDir_F);
    vec3 viewDir = normalize(viewDir_MV_F);
    vec3 normal = normalize(normal_F);
    float _u = gl_FragCoord.x / window_size.x;
    float _v = 1.0 - gl_FragCoord.y / window_size.y;
    float visibility = 0;

    vec3 rippleNormal = texture(normalMap, (uv_F + vec2(0.0, 0.005 * time))* 8.0).rgb * 2.0 - 1.0f;
    rippleNormal = vec3(rippleNormal.x, rippleNormal.z, -rippleNormal.y);
    vec3 completeNormal = normalize(normal + 0.3 * rippleNormal);

    vec3 normal_MV = normalize((NORMALM * vec4(completeNormal, 1.0)).xyz);
    float cosNL = dot(normal_MV, lightDir);
    float bias = 0.005*tan(acos(max(0, cosNL)));
    bias = 0.005 + clamp(bias, 0.0f, 0.01f);

    // generate random rotation angle for each fragment
    float angle = randomAngle(gl_FragCoord.xyz, 15);
    float s = sin(angle);
    float c = cos(angle);
    float PCFRadius = 1/300.0;
    for(int i=0; i < numSamplingPositions; i++)
    {
      // rotate offset
      vec2 rotatedOffset = vec2(kernel[i].x * c + kernel[i].y * -s, kernel[i].x * s + kernel[i].y * c);
      vec3 samplingPos = shadowCoord_F.xyz;
      samplingPos += vec3(rotatedOffset * PCFRadius, -bias);
      visibility += texture(shadowMap, samplingPos / shadowCoord_F.w);
    }
    visibility /= numSamplingPositions;

    //Flat normal is the projection of the wave normal onto the mirror surface
    vec3 flatNormal = completeNormal - dot(completeNormal, Y) * Y;
    //Compute how the flat normal look in camera space
    vec3 eyeNormal = (NORMALM * vec4(flatNormal, 1.0)).xyz;
    //Compute distortion
    vec2 reflectOffset = normalize(eyeNormal.xy) * length (flatNormal) * 0.3;

    vec3 reflection = texture(mirrorMap, vec2(_u, _v) + reflectOffset).rgb;
    vec3 lightingResult = (reflection * La);

    if(cosNL > 0){
        vec3 reflectionDir = normalize(2.0 * normal_MV * cosNL - lightDir);
         lightingResult += visibility *
                ((vec3(0.2, 0.2, 0.2) * cosNL * Ld)
                +
                (vec3(1.0, 1.0, 1.0) * pow(max(0, dot(reflectionDir, viewDir)), 512) * Ls));
    }

    float reflectionAlpha = mix(0.95, 0.5,
                                (dot(viewDir, normal_MV) - cosWaterReflectionAngle) / (1.0 - cosWaterReflectionAngle)
                                *
                                (waterReflectionDistance - length(vpoint_MV_F)));

    color = vec4(lightingResult, reflectionAlpha);
    //color = texture(normalMap, (uv_F + vec2(0.0, 0.01 * time))* 5.0);
}
