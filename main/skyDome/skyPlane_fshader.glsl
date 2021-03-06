#version 410 core
in vec2 uv;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform float time;
uniform vec2 stretchCoeff;
uniform vec3 raymarchDirection;
uniform vec3 La, Ld, Ls;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

const float alphaThreshold = 0.0f;
const float edgeSmoothingStart = 0.35f;
const float edgeSmoothingEnd = 0.49f;
const float minClamp = 0.1f;
const float maxClamp = 0.5f;
const vec2 uvCenter = vec2(0.5, 0.5);
const vec3 brightnessTreshold = vec3(1.0, 1.0, 1.0);

float expIncrease(in float v){

    return exp(4.0f * (v - 1.0f));

}

float expSmooth(in float v){

return 1.0 - exp( -pow(4.0 * v , 2.0));

}

float textureLookup(in vec2 pos){
    return texture(colorTex, pos * stretchCoeff + vec2(0.0, 0.005) * time).x;
}

float raymarchCoeff(){
    float totalCoeff = 0.0f;

    for(int i = 1; i < 5; i++){
        float depthVal = texture(colorTex, (uv + i * 0.0025 * raymarchDirection.xz)).x;

        if(depthVal > i * raymarchDirection.y){
            totalCoeff += 1;
        } else {
            break;
        }
    }

    return 1.0 - totalCoeff / 4.0f;
}

void main() {

    float distanceToCenter = length(uv - uvCenter);

    float fadeCoeff = 1.0 - smoothstep(edgeSmoothingStart, edgeSmoothingEnd, distanceToCenter);

    float depthVal = textureLookup(uv);

    float alphaVal = min(fadeCoeff, smoothstep(0.8, 1.0, depthVal));

    vec3 tmpColor = mix(vec3(1.0f), 2.0 * La + Ld, smoothstep(0.0f, 1.0, depthVal));

    color = vec4(tmpColor, clamp(alphaVal, 0.0f, 1.0f));

    float brightness = dot(color.rgb, brightnessTreshold);

    brightColor = mix(vec4(0.0, 0.0, 0.0, color.a), vec4(color), smoothstep(1.5, 6.0, brightness));
}
