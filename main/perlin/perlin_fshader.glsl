#version 330 core
in vec2 uv;
out vec3 color;

int permutation[256] = int[](
    151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
);
int p[512];
void p512(){
    for(int i = 0; i < 512 ; i ++){
        p[i] = permutation[i%256];
    }
}

//bon
float fade(float t){
  return t * t * t * (t * (t * 6 - 15) + 10);
}

//bon
float lerp(float a, float b, float x) {
                return a + x * (b - a);
        }
//bon
float grad(int hash, float x, float y)
{
    switch(hash & 0x7)
    {
        case 0x0: return  x + y;
        case 0x1: return -x + y;
        case 0x2: return  x - y;
        case 0x3: return -x - y;
        case 0x4: return  x + y;
        case 0x5: return  -x + y;
        case 0x6: return x - y;
        case 0x7: return -x - y;
        default: return 0.f; // never happens
    }
}

float pnoise(float x, float y){

    vec2 pxy = vec2(x,y),
         bottomLeft = vec2(int(x), int(y));

    int xi = int(bottomLeft.x),
        yi = int(bottomLeft.y);

    float xf = mod(x, 1),
          yf = mod(y, 1);

    int aa = p[ p[ xi    ] + yi       ],
        ab = p[ p[ xi + 1] + yi       ],
        ba = p[ p[ xi    ] + yi + 1   ],
        bb = p[ p[ xi + 1] + yi + 1   ];

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
/*
    float s = grad(aa, xf, yf),
          t = grad(ab, xf + 1, yf),
          u = grad(ba, xf, yf + 1),
          v = grad(bb, xf + 1, yf + 1);

    float st = lerp(s, t, fade(xf)),
          uv = lerp(u, v, fade(yf));


    return lerp(st, uv, fade(yf));
    */
}


float octPNoise(float x, float y, int octaves, float persistence) {
                float total = 0;
                float frequency = 2;
                float amplitude = 1;
                float maxValue = 0;			// Used for normalizing result to 0.0 - 1.0
                for(int i = 0; i < octaves; i++) {
                        total += pnoise(x * frequency, y * frequency) * amplitude;

                        maxValue += amplitude;

                        amplitude *= persistence;
                        frequency *= 1.85;
                }
                return total/maxValue + 0.5;
        }


void main() {

  p512();
  color = vec3(octPNoise(uv.x, uv.y, 8, 0.45));

}
