#version 410 core
layout (location = 0) in vec3 vpoint;
layout (location = 7) in vec2 bladeTranslation;
layout (location = 3) in mat4 instanceMatrix;

//in vec3 vpoint;
in vec2 vtexcoord;
in vec2 gridPos;
out vec2 uv;
out vec4 heightColor;
out vec2 vpoint_World_F;

uniform mat4 VP;
uniform vec2 translation;
uniform vec2 translationToSceneCenter;
uniform sampler2D heightMap;
uniform sampler2D grassMap;

const float SAND_HEIGHT = 0.02f,
            GRASS_HEIGHT = 0.3f,
            ROCK_HEIGHT = 0.40f;
const float bushHeight = 0.08;

void main() {
    vec2 coord = (bladeTranslation + vec2(1.0f, 1.0f)) * 0.5f;
    coord.y = 1.f - coord.y;
    float height = texture(heightMap, coord).x;
    float grass_coef_noise = clamp(texture(grassMap, coord).g, 0.f, 1.f);
    float ground_threshold = 0.6;

    // early culling when the vertex is not inside the grass altitude range
    if (height < SAND_HEIGHT || ROCK_HEIGHT < height ||
            //if the coefficient found in grassMap is too big -> cull
            grass_coef_noise > ground_threshold) {
        gl_Position = vec4(0, 0, 0, 0); //(0,0) is outside frustrum
        vpoint_World_F = vec2(0, 0);
        return;
    }

    vec4 vertexModelPos = (instanceMatrix * vec4(vpoint, 1.0) + vec4(translation.x, height + bushHeight/2.f, -translation.y, 0));
    gl_Position = VP * vertexModelPos;
    uv = vtexcoord;
    heightColor = vec4(vec3(height) + 0.2, 1.f);
    vpoint_World_F = translationToSceneCenter + bladeTranslation;
}
