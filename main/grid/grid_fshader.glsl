#version 330

uniform vec3 La, Ld, Ls;
uniform vec3 ka, kd, ks;
uniform float alpha;
uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform sampler2D heightMap;
uniform sampler2D normalMap;
uniform vec2 zoomOffset;
uniform float zoom;
uniform bool mirrorPass;

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

const vec3  WATER_COLOR_DEEP = vec3(14,48,150),
            WATER_COLOR = vec3(125,186,217),
            SAND_COLOR = vec3(189,173,94),
            GRASS_COLOR = vec3(52,103,0),
            ROCK_COLOR = vec3(140,140,140),
            SNOW_COLOR = vec3(231,249,251);

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
            heightCol = mix(WATER_COLOR_DEEP, WATER_COLOR, (vheight_F) / (WATER_HEIGHT));

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

    vec3 lightingResult = (heightCol * La);

    if(cosNL > 0){
         vec3 reflectionDir = normalize( 2.0 * normal_MV * cosNL - lightDir);
         lightingResult +=
                (vec3(0.7,0.7,0.7) * cosNL * Ld)
                +
                (heightCol * pow(max(0, dot(reflectionDir, viewDir_F)), 256) * Ls);
    }

    color = vec4(lightingResult, 1.0);
}
