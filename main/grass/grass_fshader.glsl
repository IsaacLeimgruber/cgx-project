#version 410 core
in vec2 uv;
in vec2 vpoint_World_F;
in vec3 lightDir;
out vec4 color;

uniform mat4 NORMALM;
uniform float max_vpoint_World_F;
uniform float threshold_vpoint_World_F;
uniform sampler2D grassAlpha;
uniform sampler2D heightMap;
uniform vec3 La, Ld;

const float alpha_threshold = 0.2;

void main() {

    //discard fragment if it's alpha value is under some threshold
   vec4 color_value = texture(grassAlpha, vec2(uv.x, 1.05f - uv.y));
    if(color_value.a < alpha_threshold){
        discard;
    }

    //compute normal_MV
    vec2 normalDxDy = texture(heightMap, uv).yz;
    vec3 gridNormal = normalize(vec3(-normalDxDy.x, 1, +normalDxDy.y));
    vec3 normal_MV = (NORMALM * vec4(gridNormal, 1.0f)).xyz;


    float cosNL = dot(normal_MV, lightDir);
    vec3 lightingResult = color_value.xyz * La;
    float visibility = 0.f;

    if(cosNL > 0.0f){
         lightingResult +=
                 visibility * (color_value.xyz * cosNL * Ld);

    }


    color = color_value;// vec4(lightingResult, 1.f);

    //fragments disappear as they come close to the fog at the edges of the terrain
    color.a *= 1 - smoothstep(threshold_vpoint_World_F, max_vpoint_World_F,
                              max(abs(vpoint_World_F.x), abs(vpoint_World_F.y))
                              );
}
