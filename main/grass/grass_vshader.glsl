#version 410 core
layout (location = 0) in vec3 vpoint;
layout (location = 7) in vec2 bladeTranslation;
layout (location = 3) in mat4 instanceMatrix;

//in vec3 vpoint;
in vec2 vtexcoord;
out vec2 uv;
out vec4 heightColor;
out vec2 vpoint_World_F;

in vec2 gridPos;

uniform mat4 VP;
uniform vec2 translation;
uniform vec2 translationToSceneCenter;
uniform sampler2D heightMap;
uniform sampler2D grassMap;

const float SAND_HEIGHT = 0.02f,
            GRASS_HEIGHT = 0.3f,
            ROCK_HEIGHT = 0.40f;

void main() {
    vec2 coord = (bladeTranslation + vec2(1.0f, 1.0f)) * 0.5f;
    coord.y = 1-coord.y;
    float height = texture(heightMap, coord).x;

    // early culling when the vertex is not inside the grass altitude range
    if (height < SAND_HEIGHT || ROCK_HEIGHT < height) {
        gl_Position = vec4(0, 0, 0, 0); //(0,0) is outside frustrum
        vpoint_World_F = vec2(0, 0);
        return;
    }

    mat4 model = instanceMatrix;
    model[3][1] += height;
    vec4 vertexModelPos = (model * vec4(vpoint, 1.0) + vec4(translation.x, 0, -translation.y, 0));
    gl_Position = VP * vertexModelPos;
    uv = vtexcoord;
    heightColor = vec4(vec3(height) + 0.2, 1.f);
    vpoint_World_F = translationToSceneCenter + bladeTranslation;
}
