#version 410 core
in vec2 uv;
uniform sampler2D colorTex;
uniform sampler2D heightMap;
in vec4 heightColor;
out vec4 color;

void main() {
    vec4 color_value = texture(colorTex, uv);
    if(color_value.a < 0.1){
        color = vec4(0.f);
    }
    //color = texture(heightMap, uv);
    //color = color_value;
    color = heightColor;
    //color = vec4(1.f, 0.f, 0.f, 1.f);
}
