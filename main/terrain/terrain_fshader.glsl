#version 410 core

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform sampler2DShadow shadowMap;
uniform sampler2D heightMap;
uniform sampler2D grassTex;
uniform sampler2D snowTex;
uniform sampler2D sandTex;
uniform sampler2D rockTex;
uniform vec3 La, Ld, Ls;
uniform bool mirrorPass;

in vec4 shadowCoord_F;
in vec4 vpoint_MV_F;
in vec4 vpoint_F;
in vec3 lightDir_F;
in vec3 viewDir_F;
in vec2 uv_F;
in float vheight_F;

out vec4 color;

const float SLOPE_THRESHOLD = 0.5f;
const float MIX_SLOPE_THRESHOLD = 0.2f;

const float WATER_HEIGHT = 0.01f,
            WATER_HEIGHT_DEEP = -0.3f,
            SAND_HEIGHT = 0.02f,
            GRASS_HEIGHT = 0.3f,
            ROCK_HEIGHT = 0.40f,
            SNOW_HEIGHT = 1.0f;

const float GRASS_TRANSITION = SAND_HEIGHT + (1.0f/5.0f) * (GRASS_HEIGHT - SAND_HEIGHT);

const int numSamplingPositions = 9;
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

const float fogStart = 2.5f;
const float fogEnd = 5.0f;

vec3 applyFog( in vec3  rgb,       // original color of the pixel
               in float distance,  // camera to point distance
               in vec3 rayDir,
               in vec3 sunDir)
{
    float d = clamp( (distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    float fogAmount = clamp(1.0 - exp(-d * 3.0), 0.0, 1.0);
    float rgbAmount = clamp(exp(-d * 2.0), 0.0, 1.0);
    float sunAmount = max( dot( rayDir, sunDir ), 0.0 );
    vec3  fogColor  = mix( vec3(0.8,0.8,0.8), // greyish
                           vec3(1.0,0.9,0.7), // yellowish
                           pow(sunAmount,16.0) );
    return rgb * rgbAmount + fogColor * fogAmount;
}


void main() {

    if(mirrorPass){
        if(vpoint_F.y < 0.0f){
            discard;
        }
    }

    vec2 normalDxDy = texture(heightMap, uv_F).yz;
    vec3 gridNormal = normalize(vec3(-normalDxDy.x, 1, +normalDxDy.y));
    vec3 normal_MV = (NORMALM * vec4(gridNormal, 1.0f)).xyz;

    vec3 lightDir = normalize(lightDir_F);

    vec3 heightCol = vec3(0.0f);
    vec3 GRASS_COLOR = 255.0 * texture(grassTex, (uv_F) * 60.0f).rgb;
    vec3 ROCK_COLOR = 255.0 * texture(rockTex, (uv_F) * 5.0f).rgb;
    vec3 WATER_COLOR = ROCK_COLOR ;
    vec3 WATER_COLOR_DEEP = vec3(25.0f,66.0f,167.0f);
    vec3 SAND_COLOR = 255.0 * texture(sandTex, (uv_F) * 30.0f).rgb;
    vec3 SNOW_COLOR = 255.0 * texture(snowTex, (uv_F) * 60.0f).rgb;
    vec3 vert = vec3(0.0f, 1.0f, 0.0f);
    float slope = dot(gridNormal, vert);//range [-1, 1], highest slope when 0

        if(vheight_F <= WATER_HEIGHT){
            heightCol = mix(SAND_COLOR, WATER_COLOR_DEEP, (vheight_F) / (WATER_HEIGHT_DEEP));

        } else if(vheight_F > WATER_HEIGHT && vheight_F <= SAND_HEIGHT){
            heightCol = SAND_COLOR;
        } else if(vheight_F > SAND_HEIGHT && vheight_F <= GRASS_HEIGHT){


            float mixCoeff = 1.0f;

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


            float x = 1.0f - (abs(slope)/SLOPE_THRESHOLD);// triangle centered in 0,
                                                        //maximum at 0 at y = 1, corners at -1 and 1
            if(slope < -MIX_SLOPE_THRESHOLD){
                heightCol = mix(heightCol, ROCK_COLOR, (-2.0f*x));
            }else if(slope > MIX_SLOPE_THRESHOLD){
                heightCol = mix(heightCol, ROCK_COLOR, 2.0f*x);
            }else{
                heightCol = ROCK_COLOR;
            }
            //heightCol = mix(heightCol, ROCK_COLOR, -TWEAK_SLOPE_MIX*x*(x - 1));
        }


    heightCol /= 255.0f;
    float cosNL = dot(normal_MV, lightDir);
    float bias = max(0.05f * (1.0f - cosNL), 0.005f);

    float visibility = 0;

    // generate random rotation angle for each fragment
    float angle = randomAngle(vpoint_F.xyz, 15.0f);
    float s = sin(angle);
    float c = cos(angle);
    float PCFRadius = 1.0f/1200.0f;
    for(int i=0; i < numSamplingPositions; i++)
    {
      // rotate offset
      vec2 rotatedOffset = vec2(kernel[i].x * c + kernel[i].y * -s, kernel[i].x * s + kernel[i].y * c);
      vec3 samplingPos = shadowCoord_F.xyz;
      samplingPos += vec3(rotatedOffset * PCFRadius, -bias);
      visibility += texture(shadowMap, samplingPos);
    }
    visibility /= numSamplingPositions;

    vec3 lightingResult = (heightCol * La);

    if(cosNL > 0.0f){
         vec3 reflectionDir = normalize( 2.0f * normal_MV * cosNL - lightDir);
         lightingResult +=visibility *
                ((heightCol * cosNL * Ld)
                +
                (vec3(0.1f,0.1f,0.1f) * pow(max(0, dot(reflectionDir, viewDir_F)), 256) * Ls));
    }

    //lightingResult = applyFog(lightingResult, length(vpoint_MV_F.z), -viewDir_F, lightDir);
    color = vec4(clamp(lightingResult, vec3(0.0f), vec3(1.0f)), 1.0f);
}
