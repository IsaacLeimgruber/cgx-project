#version 330
in vec2 position;

out vec2 uv;

uniform mat4 MVP;
uniform float time;

float freq = 3.0f/2.0f;
float amp = 0.15f;
float phi = 2.0f;

const float DEGTORAD = 3.14159265359f / 180.0f;

float freqs[4] = float[4](3.0f/2.0f, 2.0, 5.0/2.0, 20.0f);
float amps[4] =  float[4](0.04f, 0.024f, 0.01f, 0.003f);
float phis[4] = float[4](2.0f, 4.0f, 5.0f, 8.0f);
float rots[4] = float[4](0.0, DEGTORAD * 30, DEGTORAD * 30.0, DEGTORAD * 15.0);
float fades[4] = float[4](0.0, 1.0/5.0, 1.0/5.0, 2.0);
float sinWave[4] = float[4](0, 0, 0, 0);

void main() {
    uv = (position + vec2(1.0, 1.0)) * 0.5;

    // convert the 2D position into 3D positions that all lay in a horizontal
    // plane.
    // TODO 6: animate the height of the grid points as a sine function of the
    // 'time' and the position ('uv') within the grid.


    float height = 0.0;

    vec3 pos_3d = vec3(position.x, height, -position.y);

    gl_Position = MVP*vec4(pos_3d, 1.f);
}
