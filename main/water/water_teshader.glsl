#version 410 core

layout(quads, fractional_even_spacing, ccw) in;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 NORMALM;
uniform mat4 SHADOWMVP;

uniform vec2 zoomOffset;
uniform float zoom;
uniform vec3 lightPos;
uniform sampler2D normalMap;
uniform sampler2D heightMap;
uniform sampler2D mirrorTexture;

uniform float time;

in vec3 vpoint_TE[];
in vec2 uv_TE[];
in vec2 terrainGradient_TE[];
in float terrainHeight_TE[];

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
const vec3 Y = vec3(0.0, 1.0f, 0.0f);

float freqs[5] = float[5](15.0f, 20.0f, 23.0f, 30.0f, 40.0f);
float amps[5] =  float[5](0.01f, 0.0055f, 0.0045f, 0.003f, 0.002f);
float phis[5] = float[5](1.0f, 1.3f, 1.4f, 1.8f, 2.1f);
vec2 dirs[5] = vec2[5](vec2(0.0f,1.0f),vec2(0.5f, 1.0f),vec2(0.3f, 1.0f),vec2(0.4f, 1.0f),vec2(-0.2f, 1.0f));
int exps[5] = int[5](1, 2, 2, 2, 3);
float sinWave[5] = float[5](0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
float ddx[5] = float[5](0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
float ddy[5] = float[5](0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

float interpolate2D(in float v0, in float v1, in float v2, in float v3)
{
    float xlerp1 = mix(v0, v1, gl_TessCoord.x);
    float xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

vec2 interpolate2D(in vec2 v0, in vec2 v1, in vec2 v2, in vec2 v3)
{
    vec2 xlerp1 = mix(v0, v1, gl_TessCoord.x);
    vec2 xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

vec3 interpolate3D(in vec3 v0, in vec3 v1, in vec3 v2, in vec3 v3)
{
    vec3 xlerp1 = mix(v0, v1, gl_TessCoord.x);
    vec3 xlerp2 = mix(v3, v2, gl_TessCoord.x);

    return mix(xlerp1, xlerp2, gl_TessCoord.y);
}

void main()
{

    for(int i = 0; i < 5; i++){
        dirs[i] = normalize(dirs[i]);
        amps[i] *= 1.5f;
    }

    // Interpolate the attributes of the output vertex using the barycentric coordinates
    uv_F = interpolate2D(uv_TE[0], uv_TE[1], uv_TE[2], uv_TE[3]);
    float tHeight = interpolate2D(terrainHeight_TE[0], terrainHeight_TE[1], terrainHeight_TE[2], terrainHeight_TE[3]);
    vec2 tGradient = interpolate2D(terrainGradient_TE[0], terrainGradient_TE[1], terrainGradient_TE[2], terrainGradient_TE[3]);
    vpoint_F = interpolate3D(vpoint_TE[0], vpoint_TE[1], vpoint_TE[2], vpoint_TE[3]);

    for(int i = 0; i < 5; i++){

        amps[i] *=
                1.3 - 1.0 *(
                exp(-pow(5.0 * clamp(tHeight, -0.5, 0.0), 2.0)));

        float waveParam = (dot(dirs[i], vec2(vpoint_F.xz)) * freqs[i]) + (phis[i] * time);

        //Bring sin in [0,1] for later exponentiation
        float sinTmp = (sin(waveParam) + 1.0f)/2.0f;

        //First sin^k, then bring back to [-1, 1] and multiply by amplitude
        sinWave[i] = amps[i] * pow(sinTmp, exps[i]);

        //Compute derivate of the wave
        float commonPartialDerivative = exps[i]  * freqs[i] * amps[i] * pow(sinTmp, exps[i] - 1.0f) * cos(waveParam);
        ddx[i] = dirs[i].x * commonPartialDerivative;
        ddy[i] = dirs[i].y * commonPartialDerivative;
    }

    vec3 waveNormal = vec3(0.0f);
    for(int i = 0; i < 5; i++){
        vpoint_F.y += sinWave[i];
        waveNormal += vec3(-ddx[i], 1.0f, -ddy[i]);
    }

    waveNormal = normalize(waveNormal);
    normal_F = waveNormal;

    vec4 vpoint_MV = MV * vec4(vpoint_F, 1.0f);
    // Lighting
    normal_MV_F = normalize((NORMALM * vec4(waveNormal, 1.0f)).xyz);
    lightDir_F = normalize((MV * vec4(lightPos, 1.0f)).xyz - vpoint_MV.xyz);
    viewDir_MV_F = -normalize(vpoint_MV.xyz);
    vpoint_MV_F = vpoint_MV.xyz;

    gl_Position = MVP * vec4(vpoint_F, 1.0f);
    shadowCoord_F = SHADOWMVP * vec4(vpoint_F, 1.0f);
}
