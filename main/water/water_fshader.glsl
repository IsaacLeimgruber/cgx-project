#version 410 core
uniform sampler2D heightMap;
uniform sampler2D mirrorMap;
uniform sampler2DShadow shadowMap;

uniform vec3 La, Ld, Ls;
uniform vec3 ka, kd, ks;
uniform float alpha;

uniform float time;
uniform vec2 offset;

in vec2 uv_F;
in vec2 reflectOffset_F;
in vec3 normal_F;
in vec3 normal_MV_F;
in vec3 lightDir_F;
in vec3 viewDir_F;
in vec4 gl_FragCoord;
in vec4 shadowCoord_F;

out vec4 color;

const vec3 WATER_COLOR = vec3(125,186,217) / 255.0;

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
    float _u = gl_FragCoord.x / window_size.x;
    float _v = 1.0 - gl_FragCoord.y / window_size.y;

    vec3 reflection = mix(WATER_COLOR, vec3(texture(mirrorMap, vec2(_u, _v) + reflectOffset_F).rgb), 0.5f);

    vec3 lightDir = normalize(lightDir_F);
    float cosNL = dot(normal_MV_F, lightDir);

    float bias = 0.005*tan(acos(max(0, cosNL)));
    bias = 0.005 + clamp(bias, 0,0.01);

    float visibility = 0;

    // generate random rotation angle for each fragment
    float angle = randomAngle(gl_FragCoord.xyz, 15);
    float s = sin(angle);
    float c = cos(angle);
    float PCFRadius = 1/500.0;
    for(int i=0; i < numSamplingPositions; i++)
    {
      // rotate offset
      vec2 rotatedOffset = vec2(kernel[i].x * c + kernel[i].y * -s, kernel[i].x * s + kernel[i].y * c);
      vec3 samplingPos = shadowCoord_F.xyz;
      samplingPos += vec3(rotatedOffset * PCFRadius, -bias);
      visibility += texture(shadowMap, samplingPos / shadowCoord_F.w);
    }
    visibility /= numSamplingPositions;

    vec3 lightingResult = (reflection * La);

    if(cosNL > 0){
        vec3 reflectionDir = normalize(2*normal_MV_F * cosNL - lightDir);
         lightingResult += visibility *
                ((WATER_COLOR * cosNL * Ld)
                +
                (vec3(0.7,0.7,0.7) * pow(max(0, dot(reflectionDir, viewDir_F)), 150) * Ls));
    }

    color = vec4(lightingResult, 0.8f);
}
