#version 330 core
in vec2 uv;
out vec3 color;

int side_number = 5;

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

float fade(float t){
  return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float a, float b, float x) {
                return a + x * (b - a);
        }

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
int inc(int num) {
    num++;
    if (side_number > 0) num %= side_number;

    return num;
}
float pnoise(float x, float y){

    int xi = int(mod(x *side_number, 256)),
        yi = int(mod(y *side_number, 256));

    float xf = mod(x * side_number , 1.0);
    float yf = mod(y * side_number, 1.0);

    float u = fade(xf),
          v = fade(yf);

    int aa = p[ p[ xi    ] + yi        ],
        ab = p[ p[ inc(xi)] + yi       ],
        ba = p[ p[ xi    ] + inc(yi)   ],
        bb = p[ p[ inc(xi)] + inc(yi)  ];

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
                     ),v
                );
}


float octPNoise(float x, float y, int octaves, float persistence) {
                float total = 0;
                float frequency = 1;
                float amplitude = 1;
                float maxValue = 0;			// Used for normalizing result to 0.0 - 1.0
                for(int i = 0; i < octaves; i++) {
                        total += pnoise(x * frequency, y * frequency) * amplitude;

                        maxValue += amplitude;

                        amplitude *= persistence;
                        frequency *= 3;
                }
                return total/maxValue;
        }


void main() {

  p512();
  color = vec3(/*pnoise(uv.x, uv.y) +*/
               0.5*pnoise(2*uv.x, 2*uv.y) //+
               /*0.25*pnoise(4*uv.x, 4*uv.y)*/);


}
