#version 330 core
in vec2 uv;
uniform sampler2D colorTex;
uniform sampler2D velocityTex;
out vec4 color;

const float sampleNumber = 30;
const float scale = 2.0 * 1.0 / sampleNumber;

void main() {
    /// TODO: use the velocity vector stored in velocityTex to compute the line integral
    /// TODO: use a constant number of samples for integral (what happens if you take too few?)
    /// HINT: you can scale the velocity vector to make the motion blur effect more prominent
    /// HINT: to debug integration don't use the velocityTex, simply assume velocity is constant, e.g. vec2(1,0)

    // Take opposite of velocity vector so convolution is done with pixels that precede the point instead of those
    // that lie ahead of it
    vec2 dir = -texture(velocityTex, uv).xy;

        for(int i=0; i < sampleNumber; i++){
            vec2 shift = i * scale * dir;
            color += texture(colorTex, uv + shift);
        }

    color /= sampleNumber;
}
