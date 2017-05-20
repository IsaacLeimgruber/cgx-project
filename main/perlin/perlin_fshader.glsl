#version 410 core
in vec2 uv;
out vec4 color;
uniform int p[512];
uniform vec2 pos_offset;

//
//  Perlin Noise 2D Deriv
//  Return value range of -1.0->1.0, with format vec3( value, xderiv, yderiv )
//  See: http://briansharpe.wordpress.com - https://github.com/BrianSharpe/Wombat/blob/master/Perlin2D_Deriv.glsl
vec3 Perlin2D_Deriv( vec2 P )
{
    // establish our grid cell and unit position
    vec2 Pi = floor(P);
    vec4 Pf_Pfmin1 = P.xyxy - vec4( Pi, Pi + 1.0 );

    // calculate the hash
    vec4 Pt = vec4( Pi.xy, Pi.xy + 1.0 );
    Pt = Pt - floor(Pt * ( 1.0 / 71.0 )) * 71.0;
    Pt += vec2( 26.0, 161.0 ).xyxy;
    Pt *= Pt;
    Pt = Pt.xzxz * Pt.yyww;
    vec4 hash_x = fract( Pt * ( 1.0 / 951.135664 ) );
    vec4 hash_y = fract( Pt * ( 1.0 / 642.949883 ) );

    // calculate the gradient results
    vec4 grad_x = hash_x - 0.49999;
    vec4 grad_y = hash_y - 0.49999;
    vec4 norm = inversesqrt( grad_x * grad_x + grad_y * grad_y );
    grad_x *= norm;
    grad_y *= norm;
    vec4 dotval = ( grad_x * Pf_Pfmin1.xzxz + grad_y * Pf_Pfmin1.yyww );

    //	C2 Interpolation
    vec4 blend = Pf_Pfmin1.xyxy * Pf_Pfmin1.xyxy * ( Pf_Pfmin1.xyxy * ( Pf_Pfmin1.xyxy * ( Pf_Pfmin1.xyxy * vec2( 6.0, 0.0 ).xxyy + vec2( -15.0, 30.0 ).xxyy ) + vec2( 10.0, -60.0 ).xxyy ) + vec2( 0.0, 30.0 ).xxyy );

    //	Convert our data to a more parallel format
    vec3 dotval0_grad0 = vec3( dotval.x, grad_x.x, grad_y.x );
    vec3 dotval1_grad1 = vec3( dotval.y, grad_x.y, grad_y.y );
    vec3 dotval2_grad2 = vec3( dotval.z, grad_x.z, grad_y.z );
    vec3 dotval3_grad3 = vec3( dotval.w, grad_x.w, grad_y.w );

    //	evaluate common constants
    vec3 k0_gk0 = dotval1_grad1 - dotval0_grad0;
    vec3 k1_gk1 = dotval2_grad2 - dotval0_grad0;
    vec3 k2_gk2 = dotval3_grad3 - dotval2_grad2 - k0_gk0;

    //	calculate final noise + deriv
    vec3 results = dotval0_grad0
            + blend.x * k0_gk0
            + blend.y * ( k1_gk1 + blend.x * k2_gk2 );
    results.yz += blend.zw * ( vec2( k0_gk0.x, k1_gk1.x ) + blend.yx * k2_gk2.xx );
    return results * 1.4142135623730950488016887242097;  // scale things to a strict -1.0->1.0 range  *= 1.0/sqrt(0.5)
}

vec3 perlin_freq(float freq, vec2 P) {
    vec3 noise = Perlin2D_Deriv(P * freq);
    noise.g *= freq;
    noise.b *= freq;
    return noise;
}

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
//
float snoise(vec2 pos) {
    return srnoise(pos, 0.0);
}
//
// 2-D non-tiling simplex noise with rotating gradients and analytical derivative.
// The first component of the 3-element return vector is the noise value,
// and the second and third components are the x and y partial derivatives.
//
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
vec3 sdnoise_freq(float freq, vec2 P) {
    vec3 noise = sdnoise(P * freq);
    noise.g *= freq;
    noise.b *= freq;
    return noise;
}
vec3 multiply(vec3 f1, vec3 f2) {
    return vec3(
                f1.r * f2.r,
                f1.g * f2.r + f1.r * f2.g,
                f1.b * f2.r + f1.r * f2.b
                );
}
vec3 add_constant(float constant, vec3 f) {
    return f + vec3(constant, 0, 0);
}

vec3 dfBm(vec2 P) {

    // the master octave
    vec3 f0 = perlin_freq(0.2, P);

    int octaves;
    float persistence;
    float freq;
    float freqGain;
    float amplitude;

    octaves = 8;
    persistence = 0.45;
    freq = 0.7;
    freqGain = 1.9;
    amplitude = 1;

    vec3 f = vec3(1, 0, 0);
    for(int i = 0; i < octaves; i++) {
        f += sdnoise_freq(freq, P) * amplitude;
        amplitude *= persistence;
        freq *= freqGain;
    }

    // scale by the master octave
    f = multiply(f, f0);

    return f;
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------



vec3 abs(vec3 noise) {
    return sign(noise.x) * noise;
}

vec3 swissTurbulence2(vec2 p)
{
    int octaves = 4;
    float lacunarity = 2;
    float gain = 0.5;
    float warp = 0.15;
    float freq = .5;
    float amp = 1.0;
    float maxAmp = 0;
    vec3 sum;
    vec3 master = vec3(1, 0, 0); //Perlin2D_Deriv(p);

    for (int i = 0; i < octaves; ++i) {
        vec3 noise = amp * perlin_freq(freq, p);
        sum += add_constant(1, -abs(noise));
        freq *= lacunarity;
        amp *= gain * clamp(master.x, 0, 1);
        maxAmp += amp;
    }
    return sum;
}

// A perlin based fBm
// Return vector with format vec3( value, xderiv, yderiv )
// Algorithm based on swissTurbulence at http://www.decarpentier.nl/scape-procedural-extensions
// Adapted and derivates added by Julien Harbulot
vec3 swissTurbulence(vec2 p)
{
    int octaves = 8;
    float delta = 0.000001;
    float lacunarity = 2.0;
    float gain = 0.5;
    float warp = 0.15;
    float freq = 1.0;
    float amp = 1.0;

    vec2 sumd = vec2(0.0, 0.0);
    vec2 d_sumdx = vec2(0, 0); //sumd.x derivatives
    vec2 d_sumdy = vec2(0, 0); //sumd.y derivatives

    float sum = 0;
    vec2 d_sum = vec2(0, 0); //sum derivatives

    for(int i = 0; i < octaves; ++i)
    {
        vec2 x = (p + warp * sumd) * freq;

        // tmp.x is the noise, tmp.y is its x-derivative and tmp.z is its y-derivative
        vec3 tmp = Perlin2D_Deriv(x);

        vec3 tmp_deltaX = Perlin2D_Deriv(vec2(delta, 0) + x);
        vec3 tmp_deltaY = Perlin2D_Deriv(vec2(0, delta) + x);

        //derivatives of the derivative...
        vec2 d_tmpy = vec2((tmp.y - tmp_deltaX.y) / delta,
                           (tmp.y - tmp_deltaY.y) / delta);

        vec2 d_tmpz = vec2((tmp.z - tmp_deltaX.z) / delta,
                           (tmp.z - tmp_deltaY.z) / delta);

        //evaluate noise at (p + warp * sumd(p)) * freq
        float n = tmp.x;

        //derivative of the evaluation: n' = (freq + freq * warp * sumd'(p)) * n'
        vec2 d_n = freq * (1 + warp * (d_sumdx + d_sumdy)) * tmp.yz;

        //derivatives of the derivative...
        vec2 d_nx = vec2((tmp.y - tmp_deltaX.y) / delta,
                         (tmp.y - tmp_deltaY.y) / delta);

        vec2 d_ny = vec2((tmp.z - tmp_deltaX.z) / delta,
                         (tmp.z - tmp_deltaY.z) / delta);


        float sgn = sign(n);

        //increment and its derivative
        float increment = amp * (1 - sgn * n);
        vec2 d_increment = amp * -sgn * d_n;

        // update the sum and its derivative
        sum += increment;
        d_sum += d_increment;

        //compute another increment for sumd
        vec2 incrementd = -amp * tmp.yz * n;
        vec2 d_incrementdx = -amp * ((d_tmpy.x + d_tmpz.x) * n + d_n * d_n.x);
        vec2 d_incrementdy = -amp * ((d_tmpy.y + d_tmpz.y) * n + d_n * d_n.y);

        // update sumd and its derivative
        sumd += incrementd;
        d_sumdx += d_incrementdx;
        d_sumdy += d_incrementdy;

        // update parameters
        freq *= lacunarity;
        amp  *= gain * clamp(sum, 0, 1);
    }

    float correction_factor = .3;
    return vec3(sum, correction_factor * d_sum);
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

void main() {

    float scale = 2;
    float freq = 1;
    color = vec4(scale * dfBm(freq * (uv + pos_offset)), 1.0f); /*

    vec3 noise = swissTurbulence((uv + pos_offset));
    vec3 offset = vec3(-1, 0, 0);
    float scale = 1;
    //scale to ensure everything is btwn 0 and 1
    //noise.y =
    color = vec4(scale * noise + offset,  1.0f);
    //*/
}
