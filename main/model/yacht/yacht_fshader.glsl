#version 410 core

in vec2 TexCoords;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

uniform sampler2D texture_diffuse1;
uniform vec3 diffuse_color;

void main()
{
    color = vec4(diffuse_color, 1.0);
    brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
