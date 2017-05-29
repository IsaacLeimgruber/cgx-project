#version 410 core
layout (location = 0) in vec3 vpoint;
layout (location = 7) in vec2 bladeTranslation;

in vec2 vtexcoord;
in vec2 gridPos;
out vec2 uv;
out vec2 vpoint_World_F;
out vec3 lightDir;

uniform float time;
uniform mat4 V;
uniform mat4 MV;
uniform mat4 VP;
uniform vec2 translation;
uniform vec2 translationToSceneCenter;
uniform sampler2D heightMap;
uniform sampler2D grassMap;

const float SAND_HEIGHT = 0.25f,
GRASS_HEIGHT = 0.4f,
ROCK_HEIGHT = 0.5f;
const float ground_threshold = 0.6;

void main() {



    //normalize translation coordinates
    vec2 coord = (bladeTranslation + vec2(1.0f, 1.0f)) * 0.5f;

    //y axis from translation is reversed from the texture one
    coord.y = 1.f - coord.y;

    //lookup height value and grass_noise map value
    float height = texture(heightMap, coord).x;
    float grass_coef_noise = clamp(texture(grassMap, coord).g, 0.f, 1.f);

    // early culling when the vertex is not inside the grass altitude range
    if (height < SAND_HEIGHT || ROCK_HEIGHT < height ||
            //if the coefficient found in grassMap is too big -> cull
            grass_coef_noise > ground_threshold) {

        gl_Position = vec4(0, 0, 0, 0); //(0,0) is outside frustrum
        vpoint_World_F = vec2(0, 0);
        return;
    }
    //compute the model matrix (which is only translations) from the bladeTranslations
    mat4 model = mat4(1.f);
    model[3][0] = bladeTranslation.x;
    model[3][2] = bladeTranslation.y;

    float displacementX = 0.007* abs(sin(3. * time * bladeTranslation.x)) +
            0.03*abs(sin(2. * time * bladeTranslation.x + 0.2)) +
            0.04 * abs(sin(time*bladeTranslation.x / 5.f))+ 0.03;
    float displacementY = 0.f;//0.01*sin(time * bladeTranslation.y);
    if(gl_VertexID == 2  ||
            gl_VertexID == 3 ||
            gl_VertexID == 4  ||
            gl_VertexID == 8  ||
            gl_VertexID == 9  ||
            gl_VertexID == 10  ||
            gl_VertexID == 14  ||
            gl_VertexID == 15  ||
            gl_VertexID == 16  ){
        model[3][0] = bladeTranslation.x + displacementX;
        model[3][2] = bladeTranslation.y + displacementY;

    }
    vec4 vertexModelPos = (model * vec4(vpoint, 1.0) + vec4(translation.x, height - 0.03, -translation.y, 0));

    vertexModelPos = (model * vec4(vpoint, 1.0) + vec4(translation.x, height - 0.03, -translation.y, 0));

    gl_Position = VP * vertexModelPos;

    uv = vtexcoord;
    vpoint_World_F = translationToSceneCenter + bladeTranslation;

}
