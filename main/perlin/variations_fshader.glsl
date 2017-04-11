#version 330 core
in vec2 uv;
out vec3 color;

uniform int permutations[512];
uniform int octaves;
uniform float frequency_for_first_octave;
uniform float frequency_gain_per_octave;
uniform float amplitude_of_first_octave;
uniform float amplitude_gain_per_octave;

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}
float lerp(float a, float b, float x) {
    return a + x * (b - a);
}
float grad(int hash, float x, float y) {
    switch(hash & 0x7) {
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

    int aa = permutations[ permutations[xi    ] + yi    ],
        ab = permutations[ permutations[xi + 1] + yi    ],
        ba = permutations[ permutations[xi    ] + yi + 1],
        bb = permutations[ permutations[xi + 1] + yi + 1];

    float u = fade(xf),
          v = fade(yf);

    return lerp(lerp( grad(aa, xf, yf    ), grad(ab, xf - 1, yf    ), u ),
                lerp( grad(ba, xf, yf - 1), grad(bb, xf - 1, yf - 1), u ),
                v);
}

float perlin_fBm(float x, float y) {
        float frequency = frequency_for_first_octave;
        float amplitude = amplitude_of_first_octave;
        float maxValue = 0; // Used for normalizing result to 0.0 - 1.0
        float noise = 0;
        for(int i = 0; i < octaves; i++) {
                noise += perlin_noise(x * frequency, y * frequency) * amplitude;
                maxValue += amplitude;
                amplitude *= amplitude_gain_per_octave;
                frequency *= frequency_gain_per_octave;
        }
        return noise;
        //return total/(2*maxValue) + 0.5;
}

float ridged_noise(float x, float y) {
    float scaling_factor = .9;
    float noise = perlin_noise(x, y);
    if (noise < 0) noise = -noise;
    if (noise > 1) noise = 1;
    noise = scaling_factor * (1 - noise);
    noise = pow(noise, 1.8);
    return 2*noise - 1; // noise is in [-1;1]
}

float ridged_fBm(float x, float y) {
        float frequency = frequency_for_first_octave;
        float amplitude = amplitude_of_first_octave;
        float maxValue = 0; // Used for normalizing result to 0.0 - 1.0
        float noise = 0;
        for(int i = 0; i < octaves; i++) {
                noise += ridged_noise(x * frequency, y * frequency) * amplitude;
                maxValue += amplitude;
                amplitude *= amplitude_gain_per_octave;
                frequency *= frequency_gain_per_octave;
        }
        return noise;
        //return total/(2*maxValue) + 0.5;
}

float ridged_multifractal_fBm(float x, float y)
{
    float H = 2.0; // good starting value is 1
    float frequency = frequency_for_first_octave;
    float signal = ridged_noise(x,y) * amplitude_of_first_octave;
    float result = signal;
    float amplitude_max = 1.0;
    float weight = 1.0;

    for(int i = 1; i < octaves; ++i) {
        /* compute weight */
        float exponent_for_octave = pow(frequency, -H);
        frequency *= frequency_gain_per_octave;

        /* increase the frequency */
        x *= frequency_gain_per_octave;
        y *= frequency_gain_per_octave;

        /* weight successive contributions by previous signal */
        weight = (signal < 0 ? -signal : signal) * amplitude_gain_per_octave;
        weight = min(1.0, weight);
        weight = max(0.0, weight);

        signal = ridged_noise(x,y);

        /* weight the contribution */
        result += signal * weight * exponent_for_octave;
        amplitude_max += weight * exponent_for_octave;
    }
    return result;
    //return result / (2 * amplitude_max) + .5; // result in [0; 1]
}

/**
 * displays a pixel in black if the value of the noise is invalid
 * the resulting img should be all white
 */
void debug_noise(float noise) {
    if (noise < -1) color = vec3(0);
    else if (noise > 1) color = vec3(0.5);
    else color = vec3(1);
}
/**
 * displays a pixel in black if the value of the fBm is invalid
 * the resulting img should be all white
 */
void debug_fBm(float fBm) {
    if (fBm < 0) color = vec3(0);
    else if (fBm > 1) color = vec3(0.5);
    else color = vec3(1);
}

void main() {
    /*
      Quelques idées pour être créatif ici :
      https://www.classes.cs.uchicago.edu/archive/2015/fall/23700-1/final-project/MusgraveTerrain00.pdf
     */

    //float n = ridged_noise(uv.x,uv.y);
    //float n = perlin_noise(uv.x,uv.y);
    //debug_noise(n);
    //color = vec3(n);

    //float fBm = perlin_fBm(uv.x, uv.y, 30, .5);
    //float fBm = ridged_fBm(uv.x + cos(uv.x)/2, uv.y + cos(uv.y)/2, 30, .3);
    float fBm = ridged_multifractal_fBm(uv.x, uv.y);
    //debug_fBm(fBm);
    color = vec3(.6 * fBm + .2);
}
