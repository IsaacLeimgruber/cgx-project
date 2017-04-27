#version 330

uniform vec3 La, Ld, Ls;
uniform vec3 ka, kd, ks;
uniform float alpha;
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform sampler2D heightMap;
uniform sampler2D normalMap;
uniform sampler2DShadow shadowMap;
uniform vec2 zoomOffset;
uniform float zoom;
uniform bool mirrorPass;

in vec4 shadowCoord_F;
in vec4 vpoint_F;
in vec3 lightDir_F;
in vec3 viewDir_F;
in vec2 uv_F;

in float vheight_F;

out vec4 color;

const float SLOPE_THRESHOLD = 0.5;
const float MIX_SLOPE_THRESHOLD = 0.2;

const float WATER_HEIGHT = 0.1,
            SAND_HEIGHT = 0.11,
            GRASS_HEIGHT = 0.3,
            ROCK_HEIGHT = 0.40,
            SNOW_HEIGHT = 1;

const float GRASS_TRANSITION = SAND_HEIGHT + (1.0/5.0) * (GRASS_HEIGHT - SAND_HEIGHT);

const vec3  WATER_COLOR_DEEP = vec3(0,8,80),
            WATER_COLOR = vec3(125,186,217),
            SAND_COLOR = vec3(189,173,94),
            GRASS_COLOR = vec3(52,103,0),
            ROCK_COLOR = vec3(140,140,140),
            SNOW_COLOR = vec3(231,249,251);


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

    if(mirrorPass){
        if(vpoint_F.y < 0.0){
            discard;
        }
    }


    vec3 gridNormal = (texture(normalMap, (uv_F+zoomOffset) * zoom).xyz * 2.0) - 1.0f;
    vec3 normal_MV = (NORMALM * vec4(gridNormal, 1.0)).xyz;

    vec3 lightDir = normalize(lightDir_F);

    vec3 heightCol = vec3(0);
    vec3 vert = vec3(0.f, 1.f, 0.f);
    float slope = dot(gridNormal, vert);//range [-1, 1], highest slope when 0

        if(vheight_F <= WATER_HEIGHT){
            heightCol = mix(WATER_COLOR_DEEP, SAND_COLOR, (vheight_F) / (WATER_HEIGHT));

        } else if(vheight_F > WATER_HEIGHT && vheight_F <= SAND_HEIGHT){
            heightCol = SAND_COLOR;
        } else if(vheight_F > SAND_HEIGHT && vheight_F <= GRASS_HEIGHT){


            float mixCoeff = 1.0;

            if(vheight_F < GRASS_TRANSITION){
                mixCoeff = (vheight_F-SAND_HEIGHT) / (GRASS_TRANSITION-SAND_HEIGHT);
            }
            heightCol = mix(SAND_COLOR, GRASS_COLOR, mixCoeff);

        } else if(vheight_F > GRASS_HEIGHT && vheight_F <= ROCK_HEIGHT){
            heightCol = mix(GRASS_COLOR, ROCK_COLOR, (vheight_F-GRASS_HEIGHT) / (ROCK_HEIGHT-GRASS_HEIGHT));
        } else if(vheight_F > ROCK_HEIGHT){
            heightCol = mix(ROCK_COLOR, SNOW_COLOR, (vheight_F-ROCK_HEIGHT) / (SNOW_HEIGHT-ROCK_HEIGHT));
    }

        if(abs(slope) < SLOPE_THRESHOLD && vheight_F > WATER_HEIGHT){


            float x = 1 - (abs(slope)/SLOPE_THRESHOLD);// triangle centered in 0,
                                                        //maximum at 0 at y = 1, corners at -1 and 1
            if(slope < -MIX_SLOPE_THRESHOLD){
                heightCol = mix(heightCol, ROCK_COLOR, (-2*x));
            }else if(slope > MIX_SLOPE_THRESHOLD){
                heightCol = mix(heightCol, ROCK_COLOR, 2*x);
            }else{
                heightCol = ROCK_COLOR;
            }
            //heightCol = mix(heightCol, ROCK_COLOR, -TWEAK_SLOPE_MIX*x*(x - 1));
        }


    heightCol /= 255.0;
    float cosNL = dot(normal_MV, lightDir);

    float bias = 0.005*tan(acos(max(0, cosNL)));
    bias = 0.005 + clamp(bias, 0,0.01);

    float visibility = 0;

    // generate random rotation angle for each fragment
    float angle = randomAngle(vpoint_F.xyz, 15);
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

    vec3 lightingResult = (heightCol * La);

    if(cosNL > 0){
         vec3 reflectionDir = normalize( 2.0 * normal_MV * cosNL - lightDir);
         lightingResult +=visibility *
                ((heightCol * cosNL * Ld)
                +
                (vec3(0.1,0.1,0.1) * pow(max(0, dot(reflectionDir, viewDir_F)), 256) * Ls));
    }

    color = vec4(lightingResult, 1.0);
}
