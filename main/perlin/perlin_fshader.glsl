#version 330 core
in vec2 uv;
out vec3 noise;

int side_number = 100;
vec2 fade(float t){
  return t * t * t * (t * (t * 6 - 15) + 10);
}

float mix(float x, float y, float a){
    return (1 - a) * x + a * y;
}

void main() {

    // Take opposite of velocity vector so convolution is done with pixels that precede the point instead of those
    // that lie ahead of it
    vec2 pxpy = mod(uv.x*side_number, 1.0), mod(uv.y * side_number, 1.0));
    vec2 a = pxpy;
    vec2 b = vec2(1 - pxpy.x, pxpy.y);
    vec2 c = vec2(1) - pxpy;
    vec2 d = vec2(pxpy.x, pxpy.y);
    vec2 ga = vec2(1);
    vec2 gb = vec2(1);
    vec2 gc = vec2(1);
    vec2 gd = vec2(1);
    float s = ga * a;
    float t = gb * b;
    float u = gc * c;
    float v = gd * d;
    float st = mix(s, t, fade(pxpy.x));
    float uv = mix(u, v, pxpy.x);

    noise = vec3(mix(st, uv, pxpy.y));
}
