#version 410 core
in vec2 uv;
in vec2 vpoint_World_F;
in vec3 lightDir;
out vec4 color;

uniform float time;
uniform float max_vpoint_World_F;
uniform float threshold_vpoint_World_F;
uniform sampler2D grassAlpha;
//uniform vec3 La, Ld;

const float alpha_threshold = 0.2;
const float radius = 10.0f;
const vec3 sunOrbitXAxis = vec3(1.0, 0.0, 0.0);
const vec3 sunOrbitYAxis = vec3(0.0, 0.958, 0.287);
const vec3 sunOrbitCenter = vec3(0.0, 0.0, 0.0);

void main() {

    vec3 La = vec3(0.1, 0.1, 0.1);
    vec3 Ld = vec3(0.4);
    float theta = 0.02 * time - 0.2;

    vec3 lightPos = sunOrbitCenter + radius * cos(theta) * sunOrbitXAxis + radius * sin(theta) * sunOrbitYAxis;

    //discard fragment if it's alpha value is under some threshold
   vec4 color_value = texture(grassAlpha, vec2(uv.x, 1.05f - uv.y));
    if(color_value.a < alpha_threshold){
        discard;
    }

    vec3 vert = vec3(0.f, 1.f, 0.f);
    float cosNL = dot(vert, normalize(lightPos));
    vec3 lightingResult = color_value.xyz * La;

    if(cosNL > 0.0f){
         lightingResult += (color_value.xyz * cosNL * Ld);

    }

    color =  vec4(lightingResult, 1.f);

    //fragments disappear as they come close to the fog at the edges of the terrain
    color.a *= 1 - smoothstep(threshold_vpoint_World_F, max_vpoint_World_F,
                              max(abs(vpoint_World_F.x), abs(vpoint_World_F.y))
                              );
}
