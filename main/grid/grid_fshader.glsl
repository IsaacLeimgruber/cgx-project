#version 330

uniform vec3 La, Ld;
uniform vec3 ka, kd;

in vec4 vpoint_mv;
in vec3 light_dir, view_dir;

in vec2 uv;
in vec3 mv_normal;

in float vheight;

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

    vec3 triangleNormal = normalize(cross(dFdx(vpoint_mv.xyz), dFdy(vpoint_mv.xyz)));

    float cosNL = dot(triangleNormal, light_dir);

    vec3 heightCol = vec3(0);

    if(vheight <= WATER_HEIGHT){
        heightCol = mix(WATER_COLOR_DEEP, WATER_COLOR, (vheight) / (WATER_HEIGHT));

    } else if(vheight > WATER_HEIGHT && vheight <= SAND_HEIGHT){
        heightCol = SAND_COLOR;
    } else if(vheight > SAND_HEIGHT && vheight <= GRASS_HEIGHT){


        float mixCoeff = 1.0;

        if(vheight < GRASS_TRANSITION){
            mixCoeff = (vheight-SAND_HEIGHT) / (GRASS_TRANSITION-SAND_HEIGHT);
        }
        heightCol = mix(SAND_COLOR, GRASS_COLOR, mixCoeff);

    } else if(vheight > GRASS_HEIGHT && vheight <= ROCK_HEIGHT){
        heightCol = mix(GRASS_COLOR, ROCK_COLOR, (vheight-GRASS_HEIGHT) / (ROCK_HEIGHT-GRASS_HEIGHT));
    } else if(vheight > ROCK_HEIGHT){
        heightCol = mix(ROCK_COLOR, SNOW_COLOR, (vheight-ROCK_HEIGHT) / (SNOW_HEIGHT-ROCK_HEIGHT));
    }

    heightCol /= 255.0;
    vec3 reflection_dir = normalize( 2.0 * triangleNormal * max(0.0, cosNL) - light_dir);
    color = (heightCol * La) + (heightCol * cosNL * Ld) ;

}
