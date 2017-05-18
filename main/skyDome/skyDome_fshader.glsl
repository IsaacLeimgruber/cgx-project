#version 410 core
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

uniform vec3 sunPos;
uniform vec3 topSkyColor;
uniform vec3 bottomSkyColor;
uniform vec3 sunColor;
uniform float domeGradBottom;
uniform float domeGradTop;

in vec3 domePos_F;

const float sunInnerRadius = 0.05f;
const float sunOuterRadius = 0.30f;
const float sunConeAngle = 0.9f;

const vec3 brightnessTreshold = vec3(1.0, 1.0, 1.0);

float expIncrease(in float v){

    return exp(4.0f * (v - 1.0f));

}

float expSmooth(in float v){

    return 1.0 - exp( -pow(2.0 * v , 2.0));

}

void main()
{
    float domeGrad = smoothstep(domeGradBottom, domeGradTop, domePos_F.y);

    vec3 tmpColor;

    float l = length(domePos_F - sunPos);

    if(l < sunInnerRadius){
        tmpColor = sunColor;
    } else {
        tmpColor = mix(bottomSkyColor, topSkyColor, smoothstep(0.0, 1.0, domeGrad));
        if(l < sunOuterRadius){
            tmpColor = mix(sunColor, tmpColor, smoothstep(sunInnerRadius, sunOuterRadius, l));
        }
    }

    color = vec4(tmpColor, 1.0);

    float brightness = dot(color.rgb, brightnessTreshold);

    brightColor = mix(vec4(0.0, 0.0, 0.0, 1.0), vec4(color), smoothstep(0.8, 9.0, brightness));
}
