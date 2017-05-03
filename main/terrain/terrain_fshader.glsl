#version 410 core

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform sampler2DShadow shadowMap;

uniform sampler2D normalMap;
uniform sampler2D grassTex;
uniform sampler2D snowTex;
uniform sampler2D sandTex;
uniform sampler2D rockTex;
uniform vec3 La, Ld, Ls;
uniform vec2 zoomOffset;
uniform float zoom;
uniform bool mirrorPass;

in vec4 shadowCoord_F;
in vec4 vpoint_F;
in vec3 lightDir_F;
in vec3 viewDir_F;
in vec2 uv_F;
in float vheight_F;

out vec4 color;

const float SLOPE_THRESHOLD = 0.5f;
const float MIX_SLOPE_THRESHOLD = 0.2f;

const float WATER_HEIGHT = 0.1f,
            SAND_HEIGHT = 0.11f,
            GRASS_HEIGHT = 0.3f,
            ROCK_HEIGHT = 0.40f,
            SNOW_HEIGHT = 1.0f;

const float GRASS_TRANSITION = SAND_HEIGHT + (1.0f/5.0f) * (GRASS_HEIGHT - SAND_HEIGHT);


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// Modulo 289, optimizes to code without divisions
float mod289(float x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 mod289v(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
// Permutation polynomial (ring size 289 = 17*17)
float permute(float x) {
  return mod289(((x*34.0)+1.0)*x);
}
vec3 permutev(vec3 x) {
  return mod289v(((x*34.0)+1.0)*x);
}
// Hashed 2-D gradients with an extra rotation.
// (The constant 0.0243902439 is 1/41)
vec2 rgrad2(vec2 p, float rot) {
  float u = permute(permute(p.x) + p.y) * 0.0243902439 + rot; // Rotate by shift
  u = fract(u) * 6.28318530718; // 2*pi
  return vec2(cos(u), sin(u));
}
//
// 2-D non-tiling simplex noise with rotating gradients,
// without the analytical derivative.
//
float srnoise(vec2 pos, float rot) {
  // Offset y slightly to hide some rare artifacts
  pos.y += 0.001;
  // Skew to hexagonal grid
  vec2 uv = vec2(pos.x + pos.y*0.5, pos.y);

  vec2 i0 = floor(uv);
  vec2 f0 = fract(uv);
  // Traversal order
  vec2 i1 = (f0.x > f0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);

  // Unskewed grid points in (x,y) space
  vec2 p0 = vec2(i0.x - i0.y * 0.5, i0.y);
  vec2 p1 = vec2(p0.x + i1.x - i1.y * 0.5, p0.y + i1.y);
  vec2 p2 = vec2(p0.x + 0.5, p0.y + 1.0);

  // Integer grid point indices in (u,v) space
  i1 = i0 + i1;
  vec2 i2 = i0 + vec2(1.0, 1.0);

  // Vectors in unskewed (x,y) coordinates from
  // each of the simplex corners to the evaluation point
  vec2 d0 = pos - p0;
  vec2 d1 = pos - p1;
  vec2 d2 = pos - p2;

  // Wrap i0, i1 and i2 to the desired period before gradient hashing:
  // wrap points in (x,y), map to (u,v)
  vec3 x = vec3(p0.x, p1.x, p2.x);
  vec3 y = vec3(p0.y, p1.y, p2.y);
  vec3 iuw = x + 0.5 * y;
  vec3 ivw = y;

  // Avoid precision issues in permutation
  iuw = mod289v(iuw);
  ivw = mod289v(ivw);

  // Create gradients from indices
  vec2 g0 = rgrad2(vec2(iuw.x, ivw.x), rot);
  vec2 g1 = rgrad2(vec2(iuw.y, ivw.y), rot);
  vec2 g2 = rgrad2(vec2(iuw.z, ivw.z), rot);

  // Gradients dot vectors to corresponding corners
  // (The derivatives of this are simply the gradients)
  vec3 w = vec3(dot(g0, d0), dot(g1, d1), dot(g2, d2));

  // Radial weights from corners
  // 0.8 is the square of 2/sqrt(5), the distance from
  // a grid point to the nearest simplex boundary
  vec3 t = 0.8 - vec3(dot(d0, d0), dot(d1, d1), dot(d2, d2));

  // Set influence of each surflet to zero outside radius sqrt(0.8)
  t = max(t, 0.0);

  // Fourth power of t
  vec3 t2 = t * t;
  vec3 t4 = t2 * t2;

  // Final noise value is:
  // sum of ((radial weights) times (gradient dot vector from corner))
  float n = dot(t4, w);

  // Rescale to cover the range [-1,1] reasonably well
  return 11.0*n;
}
//
// 2-D non-tiling simplex noise with fixed gradients,
// without the analytical derivative.
// This function is implemented as a wrapper to "srnoise",
// at the minimal cost of three extra additions.
// Note: if this kind of noise is all you want, there are faster
// GLSL implementations of non-tiling simplex noise out there.
// This one is included mainly for completeness and compatibility
// with the other functions in the file.
float snoise(vec2 pos) {
  return srnoise(pos, 0.0);
}
//
// 2-D non-tiling simplex noise with rotating gradients and analytical derivative.
// The first component of the 3-element return vector is the noise value,
// and the second and third components are the x and y partial derivatives.
vec3 srdnoise(vec2 pos, float rot) {
  // Offset y slightly to hide some rare artifacts
  pos.y += 0.001;
  // Skew to hexagonal grid
  vec2 uv = vec2(pos.x + pos.y*0.5, pos.y);

  vec2 i0 = floor(uv);
  vec2 f0 = fract(uv);
  // Traversal order
  vec2 i1 = (f0.x > f0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);

  // Unskewed grid points in (x,y) space
  vec2 p0 = vec2(i0.x - i0.y * 0.5, i0.y);
  vec2 p1 = vec2(p0.x + i1.x - i1.y * 0.5, p0.y + i1.y);
  vec2 p2 = vec2(p0.x + 0.5, p0.y + 1.0);

  // Integer grid point indices in (u,v) space
  i1 = i0 + i1;
  vec2 i2 = i0 + vec2(1.0, 1.0);

  // Vectors in unskewed (x,y) coordinates from
  // each of the simplex corners to the evaluation point
  vec2 d0 = pos - p0;
  vec2 d1 = pos - p1;
  vec2 d2 = pos - p2;

  vec3 x = vec3(p0.x, p1.x, p2.x);
  vec3 y = vec3(p0.y, p1.y, p2.y);
  vec3 iuw = x + 0.5 * y;
  vec3 ivw = y;

  // Avoid precision issues in permutation
  iuw = mod289v(iuw);
  ivw = mod289v(ivw);

  // Create gradients from indices
  vec2 g0 = rgrad2(vec2(iuw.x, ivw.x), rot);
  vec2 g1 = rgrad2(vec2(iuw.y, ivw.y), rot);
  vec2 g2 = rgrad2(vec2(iuw.z, ivw.z), rot);

  // Gradients dot vectors to corresponding corners
  // (The derivatives of this are simply the gradients)
  vec3 w = vec3(dot(g0, d0), dot(g1, d1), dot(g2, d2));

  // Radial weights from corners
  // 0.8 is the square of 2/sqrt(5), the distance from
  // a grid point to the nearest simplex boundary
  vec3 t = 0.8 - vec3(dot(d0, d0), dot(d1, d1), dot(d2, d2));

  // Partial derivatives for analytical gradient computation
  vec3 dtdx = -2.0 * vec3(d0.x, d1.x, d2.x);
  vec3 dtdy = -2.0 * vec3(d0.y, d1.y, d2.y);

  // Set influence of each surflet to zero outside radius sqrt(0.8)
  if (t.x < 0.0) {
    dtdx.x = 0.0;
    dtdy.x = 0.0;
        t.x = 0.0;
  }
  if (t.y < 0.0) {
    dtdx.y = 0.0;
    dtdy.y = 0.0;
        t.y = 0.0;
  }
  if (t.z < 0.0) {
    dtdx.z = 0.0;
    dtdy.z = 0.0;
        t.z = 0.0;
  }

  // Fourth power of t (and third power for derivative)
  vec3 t2 = t * t;
  vec3 t4 = t2 * t2;
  vec3 t3 = t2 * t;

  // Final noise value is:
  // sum of ((radial weights) times (gradient dot vector from corner))
  float n = dot(t4, w);

  // Final analytical derivative (gradient of a sum of scalar products)
  vec2 dt0 = vec2(dtdx.x, dtdy.x) * 4.0 * t3.x;
  vec2 dn0 = t4.x * g0 + dt0 * w.x;
  vec2 dt1 = vec2(dtdx.y, dtdy.y) * 4.0 * t3.y;
  vec2 dn1 = t4.y * g1 + dt1 * w.y;
  vec2 dt2 = vec2(dtdx.z, dtdy.z) * 4.0 * t3.z;
  vec2 dn2 = t4.z * g2 + dt2 * w.z;

  return 11.0*vec3(n, dn0 + dn1 + dn2);
}

//
// 2-D non-tiling simplex noise with fixed gradients and analytical derivative.
// This function is implemented as a wrapper to "srdnoise",
// at the minimal cost of three extra additions.
//
vec3 sdnoise(vec2 pos) {
  return srdnoise(pos, 0.0);
}
float fBm(vec2 P) {
        int octaves = 8;
        float persistence = 0.45;
        float total = 0;
        float frequency = 0.5;
        float amplitude = 1;
        float maxValue = 0; // Used for normalizing result to 0.0 - 1.0
        for(int i = 0; i < octaves; i++) {
                total += snoise(P * frequency) * amplitude;
                maxValue += amplitude;
                amplitude *= persistence;
                frequency *= 1.85;
        }
        return total/maxValue + 0.3;
}
//
// The first component of the 3-element return vector is the noise value,
// and the second and third components are the x and y partial derivatives.
vec3 dfBm(vec2 P) {
        int octaves = 8;
        float persistence = 0.45;
        vec3 total = vec3(0,0,0);
        vec3 noise = vec3(0,0,0);
        float frequency = 0.5;
        float amplitude = 1;
        float maxValue = 0; // Used for normalizing result to 0.0 - 1.0
        for(int i = 0; i < octaves; i++) {
                noise = sdnoise(P * frequency) * amplitude;
                noise.y *= frequency;
                noise.z *= frequency;
                total += noise;
                maxValue += amplitude;
                amplitude *= persistence;
                frequency *= 1.85;
        }
        return total/maxValue + vec3(0.3, 0, 0);
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------



const int numSamplingPositions = 9;
uniform vec2 kernel[9] = vec2[]
(
   vec2(0.95581f, -0.18159f), vec2(0.50147f, -0.35807f), vec2(0.69607f, 0.35559f),
   vec2(-0.0036825f, -0.59150f), vec2(0.15930f, 0.089750f), vec2(-0.65031f, 0.058189f),
   vec2(0.11915f, 0.78449f), vec2(-0.34296f, 0.51575f), vec2(-0.60380f, -0.41527f)
);

// generates pseudorandom number in [0, 1]
// seed - world space position of a fragemnt
// freq - modifier for seed. The bigger, the faster
// the pseudorandom numbers will change with change of world space position
float random(in vec3 seed, in float freq)
{
   // project seed on random constant vector
   float dt = dot(floor(seed * freq), vec3(53.1215f, 21.1352f, 9.1322f));
   // return only fractional part
   return fract(sin(dt) * 2105.2354f);
}

// returns random angle
float randomAngle(in vec3 seed, in float freq)
{
   return random(seed, freq) * 6.283285f;
}

void main() {

    if(mirrorPass){
        if(vpoint_F.y < 0.0f){
            discard;
        }
    }

    vec3 noise = dfBm((uv_F+zoomOffset) * zoom);
    vec3 normal = vec3(noise.y, noise.z, -1);
    vec3 gridNormal = normal;//(texture(normalMap, (uv_F+zoomOffset) * zoom).xyz * 2.0f) - 1.0f;
    vec3 normal_MV = (NORMALM * vec4(gridNormal, 1.0f)).xyz;

    vec3 lightDir = normalize(lightDir_F);

    vec3 heightCol = vec3(0.0f);
    vec3 GRASS_COLOR = 255.0 * texture(grassTex, (uv_F+zoomOffset) * zoom * 60.0f).rgb;
    vec3 ROCK_COLOR = 255.0 * texture(rockTex, (uv_F+zoomOffset) * zoom * 5.0f).rgb;
    vec3 WATER_COLOR = ROCK_COLOR;
    vec3 WATER_COLOR_DEEP = ROCK_COLOR;
    vec3 SAND_COLOR = 255.0 * texture(sandTex, (uv_F+zoomOffset) * zoom * 30.0f).rgb;
    vec3 SNOW_COLOR = 255.0 * texture(snowTex, (uv_F+zoomOffset) * zoom * 60.0f).rgb;
    vec3 vert = vec3(0.0f, 1.0f, 0.0f);
    float slope = dot(gridNormal, vert);//range [-1, 1], highest slope when 0

        if(vheight_F <= WATER_HEIGHT){
            heightCol = mix(WATER_COLOR_DEEP, SAND_COLOR, (vheight_F) / (WATER_HEIGHT));

        } else if(vheight_F > WATER_HEIGHT && vheight_F <= SAND_HEIGHT){
            heightCol = SAND_COLOR;
        } else if(vheight_F > SAND_HEIGHT && vheight_F <= GRASS_HEIGHT){


            float mixCoeff = 1.0f;

            if(vheight_F < GRASS_TRANSITION){
                mixCoeff = (vheight_F-SAND_HEIGHT) / (GRASS_TRANSITION-SAND_HEIGHT);
            }
            heightCol = mix(SAND_COLOR, GRASS_COLOR, mixCoeff);

        } else if(vheight_F > GRASS_HEIGHT && vheight_F <= ROCK_HEIGHT){
            heightCol = mix(GRASS_COLOR, ROCK_COLOR, (vheight_F-GRASS_HEIGHT) / (ROCK_HEIGHT-GRASS_HEIGHT));
        } else if(vheight_F > ROCK_HEIGHT){
            heightCol = mix(ROCK_COLOR, SNOW_COLOR, (vheight_F-ROCK_HEIGHT) / (SNOW_HEIGHT-ROCK_HEIGHT));
    }

        if(abs(slope) < SLOPE_THRESHOLD && vheight_F > WATER_HEIGHT){


            float x = 1.0f - (abs(slope)/SLOPE_THRESHOLD);// triangle centered in 0,
                                                        //maximum at 0 at y = 1, corners at -1 and 1
            if(slope < -MIX_SLOPE_THRESHOLD){
                heightCol = mix(heightCol, ROCK_COLOR, (-2.0f*x));
            }else if(slope > MIX_SLOPE_THRESHOLD){
                heightCol = mix(heightCol, ROCK_COLOR, 2.0f*x);
            }else{
                heightCol = ROCK_COLOR;
            }
            //heightCol = mix(heightCol, ROCK_COLOR, -TWEAK_SLOPE_MIX*x*(x - 1));
        }


    heightCol /= 255.0f;
    float cosNL = dot(normal_MV, lightDir);
    float bias = max(0.05f * (1.0f - cosNL), 0.005f);

    float visibility = 0;

    // generate random rotation angle for each fragment
    float angle = randomAngle(vpoint_F.xyz, 15.0f);
    float s = sin(angle);
    float c = cos(angle);
    float PCFRadius = 1.0f/300.0f;
    for(int i=0; i < numSamplingPositions; i++)
    {
      // rotate offset
      vec2 rotatedOffset = vec2(kernel[i].x * c + kernel[i].y * -s, kernel[i].x * s + kernel[i].y * c);
      vec3 samplingPos = shadowCoord_F.xyz;
      samplingPos += vec3(rotatedOffset * PCFRadius, -bias);
      visibility += texture(shadowMap, samplingPos / shadowCoord_F.w);
    }
    visibility /= numSamplingPositions;

    vec3 lightingResult = (heightCol * La);

    if(cosNL > 0.0f){
         vec3 reflectionDir = normalize( 2.0f * normal_MV * cosNL - lightDir);
         lightingResult +=visibility *
                ((heightCol * cosNL * Ld)
                +
                (vec3(0.1f,0.1f,0.1f) * pow(max(0, dot(reflectionDir, viewDir_F)), 256) * Ls));
    }

    color = vec4(lightingResult, 1.0f);
}
