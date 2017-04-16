#version 410 core
uniform sampler2D heightMap;
uniform sampler2D mirrorTex;
uniform sampler2D normalMap;

uniform vec3 La, Ld, Ls;
uniform vec3 ka, kd, ks;
uniform float alpha;

uniform float time;
uniform vec2 offset;

in vec2 uv_F;
in vec2 reflectOffset_F;
in vec3 normal_F;
in vec4 gl_FragCoord;
in vec3 normal_MV_F;
in vec3 lightDir_F;
in vec3 viewDir_F;

out vec4 color;

void main() {

    vec2 window_size = textureSize(mirrorTex, 0);
    float _u = gl_FragCoord.x / window_size.x;
    float _v = gl_FragCoord.y / window_size.y;

    vec3 reflection = mix(vec3(125,186,217) / 255.0, vec3(texture(mirrorTex, vec2(_u, _v) + reflectOffset_F).rgb), 0.5f);


    float cosNL = dot(normal_MV_F, lightDir_F);

    vec3 reflectionDir = normalize(2*normal_MV_F * cosNL - lightDir_F);

    vec3 lightingColor =   //Ambient
            (reflection * La)
            + //Diffuse
            (vec3(1,1,1) * max(0, cosNL) * Ld)
            + //Specular
            (vec3(1,1,1) * pow(max(0, dot(reflectionDir, viewDir_F)), 160) * Ls);

    color = vec4(lightingColor, 0.8f);
}
