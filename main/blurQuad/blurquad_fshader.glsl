#version 330

in vec2 uv;

out vec4 color;

uniform sampler2D tex;
uniform float tex_width;
uniform float tex_height;
uniform int blurQuantity;
uniform int second_pass;

const float[5] G = float[5](0.153388,	0.221461,	0.250301,	0.221461,	0.153388);
const float[5] linG = float[5](0.212195, 0.229344, 0.116923, 0.229344, 0.212195);
const float[5] linOffset = float[5](-3.2307692308, -1.3846153846, 0.0, 1.3846153846, 3.2307692308 );
void main() {

    int K = 2;

    vec3 tot_image = vec3(0.0);
    float tot_weight = 0;
    for(int k = -K; k <= K; k++){
        float weight = linG[k + K];
        tot_image +=
                weight
                *                 // Ternary condition to convolve either vertically or horizontally
                texture(tex, uv + ((second_pass == 0) ? vec2(linOffset[k]/tex_width, 0) : vec2(0, linOffset[k]/tex_height))).rgb;
        tot_weight += weight;
    }

    color = vec4(tot_image / tot_weight, 1.0);

}

