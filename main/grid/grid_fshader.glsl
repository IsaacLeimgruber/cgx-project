#version 330

uniform vec3 La, Ld;
uniform vec3 ka, kd;

in vec4 vpoint_MV_F;
in vec3 lightDir_F, viewDir_F;

in vec2 uv_F;

in float vheight_F;

out vec3 color;

const float WATER_HEIGHT = 0.1,
            SAND_HEIGHT = 0.11,
            GRASS_HEIGHT = 0.3,
            ROCK_HEIGHT = 0.40,
            SNOW_HEIGHT = 1;

const float GRASS_TRANSITION = SAND_HEIGHT + (1.0/5.0) * (GRASS_HEIGHT - SAND_HEIGHT);

const vec3  WATER_COLOR_DEEP = vec3(34,68,170),
            WATER_COLOR = vec3(125,186,217),
            SAND_COLOR = vec3(189,173,94),
            GRASS_COLOR = vec3(52,103,0),
            ROCK_COLOR = vec3(160,160,160),
            SNOW_COLOR = vec3(231,249,251);

void main() {

    vec3 triangleNormal = normalize(cross(dFdx(vpoint_MV_F.xyz), dFdy(vpoint_MV_F.xyz)));

    float cosNL = dot(triangleNormal, lightDir_F);

    vec3 heightCol = vec3(0);

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

    heightCol /= 255.0;
    vec3 reflection_dir = normalize( 2.0 * triangleNormal * max(0.0, cosNL) - lightDir_F);
    color = (heightCol * La) + (heightCol * cosNL * Ld) ;

}
