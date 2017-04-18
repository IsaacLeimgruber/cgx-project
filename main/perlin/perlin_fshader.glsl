#version 330 core
in vec2 uv;
out vec3 color;
uniform int p[512];
uniform float xoffset;
uniform float yoffset;

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
  // The noise is not defined when x, y are negative
  // Taking the absolute value creates a mirror effect
    x = abs(x);
    y = abs(y);

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

//float ridged_noise(float x, float y) {
//  return 1.0f - abs(perlin_noise(x, y));
//}
float ridged_noise(float x, float y) {
  float scaling_factor = .9;
  float noise = perlin_noise(x, y);
  if (noise < 0) noise = -noise;
  if (noise > 1) noise = 1;
  noise = scaling_factor * (1 - noise);
  noise = pow(noise, 1.8);
  return noise - .5; // noise is in [-1;1]
}

float fBm(float x, float y) {
        int octaves = 8;
        float persistence = .5;
        float frequency = 2;
        float amplitude = 1;
        float first = 2 * pow(perlin_noise(x,y), 3);
        float total = 1.3 * first;
        float p = (abs(first) > .8) ? 0 : 1 - abs(first);
        p = p*p;
        for(int i = 0; i < 3; i++) {
          total += perlin_noise(x * frequency, y * frequency)
            * amplitude * .1
            * p;
          amplitude *= persistence;
          frequency *= 1.4;
        }

        frequency = 2;
        amplitude = 1;
        float maxValue = 1.3; // Used for normalizing result to 0.0 - 1.0
        for(int i = 0; i < octaves; i++) {
          total += ridged_noise(x * frequency, y * frequency)
                   * amplitude
                   * first;
          maxValue += amplitude * first;
          amplitude *= persistence;
          frequency *= 1.85;
        }
        return total/maxValue + 0.5;
}

void main() {
  float f = fBm(xoffset + uv.x, yoffset + uv.y);
  f = f < -1 ? -1 : f;
  f = f > 1 ? -1 : f;
  color = vec3(f);
}
