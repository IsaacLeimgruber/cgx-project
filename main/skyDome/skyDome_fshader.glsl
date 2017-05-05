#version 410 core
in vec3 domePos_F;
out vec4 color;

void main()
{
    float domeGrad = (domePos_F.y);
    vec3 skyCol = vec3(15,173,255) / 255.0;
    vec3 whiteCol = vec3(1.0, 1.0, 1.0);
    color = vec4(mix(whiteCol, skyCol, clamp(domeGrad, 0.0, 1.0)), 1.0);
}
