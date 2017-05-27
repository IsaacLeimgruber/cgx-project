#version 410 core
in vec2 uv;
uniform sampler2D colorTex;
uniform sampler2D bloomTex;
out vec4 color;

uniform float exposure;
uniform float gamma;

void main() {

    color = texture(colorTex, uv);


    vec3 hdrColor = texture(colorTex, uv).rgb;
    vec3 bloomColor = texture(bloomTex, uv).rgb;
    vec3 result = hdrColor + bloomColor; // additive blending
    // tone mapping
    result = vec3(1.0) - exp(-result * exposure);
    //gamma correction
    result = pow(result, vec3(1.0 / gamma));
    color = vec4(result, 1.0f);

}
