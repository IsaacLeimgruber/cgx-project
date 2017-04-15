#version 330
uniform sampler2D normalMap;
in vec4 vpoint_MV_F;
in vec4 vpoint_M_F;
in vec3 lightDir_F;
in vec3 viewDir_F;
in vec2 uv_F;

in float vheight_F;

out vec4 color;

void main() {

    color = vec4(1.0,0.0,0.0,1.0);
}
