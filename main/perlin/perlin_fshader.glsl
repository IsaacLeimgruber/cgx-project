#version 330 core
in vec2 uv;
out vec4 color;
uniform int p[512];
uniform float pos_offset[2];

//bon
float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

//bon
float lerp(float a, float b, float x) {
    return a + x * (b - a);
}
//bon
float grad(int hash, float x, float y) {
    switch(hash & 0x7)
    {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + y;
        case 0x5: return -x + y;
        case 0x6: return  x - y;
        case 0x7: return -x - y;
        default: return 0.f; // never happens
    }
}

float perlin_noise(float x, float y) {

    vec2 pxy = vec2(x,y),
         bottomLeft = vec2(int(x), int(y));

    int xi = int(bottomLeft.x),
        yi = int(bottomLeft.y);

    float xf = mod(x, 1),
          yf = mod(y, 1);

    int aa = p[ p[xi    ] + yi    ],
        ab = p[ p[xi + 1] + yi    ],
        ba = p[ p[xi    ] + yi + 1],
        bb = p[ p[xi + 1] + yi + 1];

    float u = fade(xf),
          v = fade(yf);

    return lerp(
                lerp(
                    grad(aa, xf, yf),
                    grad(ab, xf - 1, yf),
                    u
                    ),
                lerp(
                    grad(ba, xf, yf - 1),
                    grad(bb, xf - 1, yf - 1),
                    u
                    ),
                v
                );
}

float octPNoise(float x, float y, int octaves, float persistence) {
        float total = 0;
        float frequency = 2;
        float amplitude = 1;
        float maxValue = 0; // Used for normalizing result to 0.0 - 1.0
        for(int i = 0; i < octaves; i++) {
                total += perlin_noise(x * frequency, y * frequency) * amplitude;
                maxValue += amplitude;
                amplitude *= persistence;
                frequency *= 1.85;
        }
        return total/maxValue + 0.5;
}

float ridged_noise(float x, float y, int octaves, float persistence) {
    return 1.0f - abs(octPNoise(x, y, octaves, persistence));
}

void main() {
    color = vec4(vec3(octPNoise(uv.x, uv.y, 8, 0.45)), 1.0f);
    //TODO: pourquoi ça ne marche pas quand on utilise le pos_offset ? J'en ai besoin !
    //color = vec3(octPNoise(uv.x + pos_offset[0], uv.y + pos_offset[1], 8, 0.45));
    //color = vec3(pos_offset[0], pos_offset[1], 0);
}
