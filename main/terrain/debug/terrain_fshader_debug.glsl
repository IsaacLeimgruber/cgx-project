#version 410 core

in vec4 vpoint_MV_F;
in vec4 vpoint_M_F;
in vec3 lightDir_F;
in vec3 viewDir_F;
in vec2 uv_F;

in float vheight_F;
in vec3 vcolor;

out vec4 color;

void main() {

    color = vec4(vcolor,1.0);
}
