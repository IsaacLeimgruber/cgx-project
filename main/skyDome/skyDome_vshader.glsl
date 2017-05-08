#version 410 core
in vec3 domePos;
out vec3 domePos_F;
uniform mat4 MVP;

void main()
{
    gl_Position =  MVP * vec4(domePos, 1.0);
    domePos_F = domePos;
}
