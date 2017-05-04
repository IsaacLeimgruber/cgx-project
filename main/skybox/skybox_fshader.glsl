#version 410 core
in vec3 TexCoords;
out vec4 color;

uniform samplerCube skybox;

void main()
{
    color = mix(texture(skybox, TexCoords), vec4(0.8, 0.8, 0.8, 1.0), pow(1.0 - clamp(TexCoords.y, 0.0, 1.0), 4.0));
}
