#version 410 core
in vec2 uv;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform float time;
out vec4 color;

const float alphaThreshold = 0.0f;
const float edgeSmoothingStart = 0.49f;
const float edgeSmoothingEnd = 0.5f;
const float minClamp = 0.1f;
const float maxClamp = 0.5f;
const vec3 raymarchDirection = vec3(0.001f, 0.05f, 0.0f);

float expIncrease(in float v){

    return exp(4.0f * (v - 1.0f));

}

float expSmooth(in float v){

return 1.0 - exp( -pow(4.0 * v , 2.0));

}

float raymarchCoeff(){
    float totalCoeff = 0.0f;

    for(int i = 1; i < 4; i++){
        float depthVal = texture(colorTex, (uv + i * raymarchDirection.xz)).x;

        totalCoeff += 1;
    }

    return 1.0 - totalCoeff / 4.0f;
}

void main() {

    vec2 centerToUv = abs(uv - 0.5f);

    float uFadeCoeff = smoothstep(edgeSmoothingStart, edgeSmoothingEnd, centerToUv.x);
    float vFadeCoeff = smoothstep(edgeSmoothingStart, edgeSmoothingEnd, centerToUv.y);


    float depthVal = texture(colorTex, uv * vec2(3.0, 2.0) + vec2(0.0, 0.002) * time).x;

    float uvFade = 1.0 - max(uFadeCoeff, vFadeCoeff);
    float alphaVal = min(uvFade, smoothstep(0.4, 0.42, depthVal));



    float colorCoeff = mix(1.0, 0.8, smoothstep(0.4, 1.0, depthVal));

    vec3 tmpColor = vec3(colorCoeff);
    color = vec4(vec3(1.0 * depthVal + 0.4), smoothstep(0.4, 0.8, depthVal));
}
