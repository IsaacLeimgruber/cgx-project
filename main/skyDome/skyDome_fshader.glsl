#version 410 core

uniform vec3 sunPos;
uniform vec3 topSkyColor;
uniform vec3 bottomSkyColor;
uniform float domeGradBottom;
uniform float domeGradTop;

in vec3 domePos_F;
out vec4 color;

const float sunInnerRadius = 0.4f;
const float sunOuterRadius = 0.5f;
const float sunConeAngle = 0.9f;
const vec3 sunInnerColor = vec3(1.0, 1.0, 1.0);
const vec3 sunBorderColor = vec3(1.0, 1.0, 0.51);

float expIncrease(in float v){

    return exp(4.0f * (v - 1.0f));

}

float expSmooth(in float v){

return 1.0 - exp( -pow(2.0 * v , 2.0));


}

void main()
{
    float domeGrad = clamp((domePos_F.y - domeGradBottom) / (domeGradTop - domeGradBottom), 0.0f, 1.0f);

    vec3 tmpColor = mix(bottomSkyColor, topSkyColor, clamp(expSmooth(domeGrad), 0.0f, 1.0f));

    float l = length(domePos_F - sunPos);

    if(l < sunInnerRadius){
        tmpColor = sunInnerColor;
    } else if(l < sunOuterRadius){
        tmpColor = mix(
                    sunInnerColor,
                    tmpColor,
                    expSmooth(
                                clamp(
                                    (l - sunInnerRadius) / (sunOuterRadius - sunInnerRadius)
                                ,0.0, 1.0)
                               )
                   );
    }

    color = vec4(tmpColor, 1.0);
}
