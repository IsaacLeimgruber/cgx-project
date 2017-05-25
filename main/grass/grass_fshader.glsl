#version 410 core
in vec2 uv;
in vec2 vpoint_World_F;
out vec4 color;

uniform float max_vpoint_World_F;
uniform float threshold_vpoint_World_F;
uniform sampler2D grassAlpha;

const float alpha_threshold = 0.2;

void main() {

    //discard fragment if it's alpha value is under some threshold
   vec4 color_value = texture(grassAlpha, vec2(uv.x, 1.05f - uv.y));
    if(color_value.a < alpha_threshold){
        discard;
    }
    color = color_value;

    //fragments disappear as they come close to the fog at the edges of the terrain
    color.a *= 1 - smoothstep(threshold_vpoint_World_F, max_vpoint_World_F,
                              max(abs(vpoint_World_F.x), abs(vpoint_World_F.y))
                              );
}
