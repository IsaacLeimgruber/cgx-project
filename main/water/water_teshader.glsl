#version 410 core

layout(quads, fractional_even_spacing, ccw) in;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform mat4 SHADOWMVP;

uniform vec2 offset;
uniform vec3 lightPos;
uniform sampler2D normalMap;
uniform sampler2D mirrorTexture;

uniform float time;

in vec3 vpoint_TE[];
in vec2 uv_TE[];

out vec2 uv_F;
out vec2 reflectOffset_F;
out vec3 vpoint_F;
out vec3 vpoint_MV_F;
out vec3 normal_F;
out vec3 normal_MV_F;
out vec3 lightDir_F;
out vec3 viewDir_MV_F;
out vec4 shadowCoord_F;

const float DEGTORAD = 3.14159265359f / 180.0f;
const vec3 Y = vec3(0.0, 1.0, 0.0);

float freqs[5] = float[5](30.0f, 60.0, 120.0, 160.0f, 180.0f);
float amps[5] =  float[5](0.002f, 0.0016f, 0.001f, 0.0007f, 0.0007f);
float phis[5] = float[5](1.8f, 2.0f, 3.0f, 5.0f, 6.5f);
vec2  dirs[5] = vec2[5](vec2(0.0,1.0),vec2(0.5, 1.0),vec2(0.3, 1.0),vec2(0.4, 1.0),vec2(-0.2, 1.0));
float exps[5] = float[5](1.0, 2.0, 2.0, 1.0, 1.0);
float fades[5] = float[5](0.0, 1.0/5.0, 1.0/5.0, 2.0, 2.0);
float sinWave[5] = float[5](0, 0, 0, 0, 0);
float ddx[5] = float[5](0, 0, 0, 0, 0);
float ddy[5] = float[5](0, 0, 0, 0, 0);

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2, vec2 v3)
{
    vec2 xlerp1 = mix(v0, v1, gl_TessCoord.x);
    vec2 xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
    vec3 xlerp1 = mix(v0, v1, gl_TessCoord.x);
    vec3 xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

void main()
{
    for(int i = 0; i < 5; i++){
       dirs[i] = normalize(dirs[i]);
    }

    // Interpolate the attributes of the output vertex using the barycentric coordinates
    uv_F = interpolate2D(uv_TE[0], uv_TE[1], uv_TE[2], uv_TE[3]);
    vec3 vpoint_F = interpolate3D(vpoint_TE[0], vpoint_TE[1], vpoint_TE[2], vpoint_TE[3]);

    for(int i = 0; i < 5; i++){

        float waveParam = (dot(dirs[i], uv_F) * freqs[i]) + (phis[i] * time);

        //Bring sin in [0,1] for later exponentiation
        float sinTmp = (sin(waveParam) + 1.0)/2.0;

        //First sin^k, then bring back to [-1, 1] and multiply by amplitude
        sinWave[i] = amps[i] * ( 2.0 * pow(sinTmp, exps[i])-1.0);

        //Compute derivate of the wave
        float commonPartialDerivative = exps[i]  * freqs[i] * amps[i] * pow(sinTmp, exps[i]-1.0) * cos(waveParam);
        ddx[i] = dirs[i].x * commonPartialDerivative;
        ddy[i] = dirs[i].y * commonPartialDerivative;
    }

    vec3 waveNormal = vec3(0.0);
    for(int i = 0; i < 5; i++){
        vpoint_F.y += sinWave[i];
        waveNormal += vec3(-ddx[i], 1.0, ddy[i]);
    }

    waveNormal = normalize(waveNormal);
    normal_F = waveNormal;

    vec4 vpoint_MV = MV * vec4(vpoint_F, 1.0);
    // Lighting
    normal_MV_F = normalize((NORMALM * vec4(waveNormal, 1.0)).xyz);
    lightDir_F = normalize((MV * vec4(lightPos, 1.0)).xyz - vpoint_MV.xyz);
    viewDir_MV_F = -normalize(vpoint_MV.xyz);
    vpoint_MV_F = vpoint_MV.xyz;

    gl_Position = MVP * vec4(vpoint_F, 1.0);
    shadowCoord_F = SHADOWMVP * vec4(vpoint_F, 1.0);
}

