#version 410

in vec2 uv_F;

out vec3 color;

uniform sampler2D mirrorMap;
uniform int second_pass;

const float[7] G = float[7](0.00038771,	0.01330373, 0.11098164,	0.22508352, 0.11098164,	0.01330373, 0.00038771);

void main() {

    vec2 texDim = textureSize(mirrorMap, 0);
    int K = 7/2;

        vec3 tot_image = vec3(0.0);
        float tot_weight = 0;
            for(int k = -K; k <= K; k++){
                float weight = G[k + K];
                    tot_image +=
                            weight
                            *                 // Ternary condition to convolve either vertically or horizontally
                            texture(mirrorMap, uv + ((second_pass == 0) ? vec2(k/texDim.x, 0) : vec2(0, k/texDim.y))).rgb;
                tot_weight += weight;
            }

        color = tot_image / tot_weight;
}

