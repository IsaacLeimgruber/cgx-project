#version 410 core
in vec2 uv;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
out vec4 color;

const float fogStart = 0.98f;
const float fogEnd = 1.0f;

void main() {

        float distance = texture(depthTex, uv).r;
        float d = clamp( (distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
        float fogAmount = clamp(1.0 - exp(-pow(d * 3.0, 2)), 0.0, 1.0);
        float rgbAmount = clamp(exp(-pow(d * 3.0, 2)), 0.0, 1.0);
        vec4  fogColor  = vec4(0.8,0.8,0.8, 1.0);

    if(distance == 1.0){
        color = texture(colorTex, uv);
    }else{
        color = texture(colorTex, uv) * rgbAmount + fogColor * fogAmount;
    }
}
