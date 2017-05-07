#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../gridmesh.h"
#include "../light/light.h"
#include "../light/lightable.h"
#include "../material/material.h"
#include "../camera/fractionalview.h"
#include "../utils.h"

using namespace glm;

class SkyDome: public ILightable{


private:
    GLuint vertex_array_id_;        // vertex array object
    GLuint program_id_;             // GLSL shader program ID
    GLuint vertex_buffer_object_position_;  // memory buffer for positions
    GLuint vertex_buffer_object_index_;     // memory buffer for indices
    GLuint MVPId;
    GLuint sunPosId, bottomSkyColorId, topSkyColorId;
    Light* light;

    const float radius = 10.0f;
    const int rings = 24;
    const int sectors = 24;

    const vec3 sunOrbitXAxis = vec3(1.0, 0.0, 0.0);
    const vec3 sunOrbitYAxis = vec3(0.0, 1.0, 0.0);
    const vec3 sunOrbitCenter = vec3(0.0, 0.0, 0.0);

    const vec3 sunsetColor = vec3(1.0f, 0.568f, 0.078f);
    const vec3 nightSkyColor = vec3(0.129f, 0.2f, 0.267f);
    const vec3 SUNSETCOL_topSky = vec3(0.298f, 0.494f, 0.741);
    const vec3 SUNSETCOL_bottomSky = vec3(0.894f, 0.533, 0.537);
    const vec3 mistColor = vec3(0.8, 0.8, 0.8);
    const vec3 blueSkyColor = vec3(0.059f, 0.678f, 1.0);

    const vec3 LIGHTCOL_NIGHT = nightSkyColor;
    const vec3 LIGHTCOL_SUNSET = SUNSETCOL_bottomSky;
    const vec3 LIGHTCOL_DAY = vec3(0.0f, 0.0f, 0.0f);

    const float sunsetEnd = 1.0f;
    const float sunsetBegin = -0.5f;
    const float nightBegin = -2.0f;

    vec3 bottomSkyColor;
    vec3 topSkyColor;

    float PI = 3.14159265359f;
    float PIovr2 = PI * 0.5f;
    float PI2 = 2.0f * PI ;

    int numIndices;
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> texcoords;
    std::vector<GLuint> indices;

public:

    void Init() {

        // compile the shaders
        program_id_ = icg_helper::LoadShaders("skyDome_vshader.glsl",
                                              "skyDome_fshader.glsl");
        if(!program_id_) {
            exit(EXIT_FAILURE);
        }

        glUseProgram(program_id_);

        glGenVertexArrays(1, &vertex_array_id_);
        glBindVertexArray(vertex_array_id_);

        //Generate sphere,
        // adapted from http://stackoverflow.com/questions/5988686/creating-a-3d-sphere-in-opengl-using-visual-c
        {
            float const R = 1./(float)(rings-1);
            float const S = 1./(float)(sectors-1);
            int r, s;

            vertices.resize(rings * sectors * 3);
            normals.resize(rings * sectors * 3);
            texcoords.resize(rings * sectors * 2);
            std::vector<GLfloat>::iterator v = vertices.begin();
            std::vector<GLfloat>::iterator n = normals.begin();
            std::vector<GLfloat>::iterator t = texcoords.begin();
            for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
                float const y = sin( -PIovr2 + PI * r * R );
                float const x = cos(PI2 * s * S) * sin( PI * r * R );
                float const z = sin(PI2 * s * S) * sin( PI * r * R );

                *t++ = s*S;
                *t++ = r*R;

                *v++ = x * radius;
                *v++ = y * radius;
                *v++ = z * radius;

                *n++ = x;
                *n++ = y;
                *n++ = z;
            }

            indices.resize(rings * sectors * 6);
            std::vector<GLuint>::iterator i = indices.begin();
            for(r = 0; r < rings-1; r++) {
                for(s = 0; s < sectors-1; s++) {
                    *i++ = r * sectors + s;
                    *i++ = (r+1) * sectors + (s+1);
                    *i++ = r * sectors + (s+1);
                    *i++ = r * sectors + s;
                    *i++ = (r+1) * sectors + s;
                    *i++ = (r+1) * sectors + (s+1);
                }
            }
        }

        numIndices = indices.size();

        // position buffer
        glGenBuffers(1, &vertex_buffer_object_position_);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_position_);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                     &vertices[0], GL_STATIC_DRAW);

        // vertex indices
        glGenBuffers(1, &vertex_buffer_object_index_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer_object_index_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                     &indices[0], GL_STATIC_DRAW);

        // position shader attribute
        GLuint loc_position = glGetAttribLocation(program_id_, "domePos");
        glEnableVertexAttribArray(loc_position);
        glVertexAttribPointer(loc_position, 3, GL_FLOAT, DONT_NORMALIZE,
                              ZERO_STRIDE, ZERO_BUFFER_OFFSET);

        // other ids
        sunPosId = glGetUniformLocation(program_id_, "sunPos");
        topSkyColorId = glGetUniformLocation(program_id_, "topSkyColor");
        bottomSkyColorId= glGetUniformLocation(program_id_, "bottomSkyColor");
        MVPId = glGetUniformLocation(program_id_, "MVP");

        // to avoid the current object being polluted
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void useLight(Light* l){
        this->light = l;
        light->registerProgram(program_id_);
    }

    void Cleanup() {
        glBindVertexArray(0);
        glUseProgram(0);
        glDeleteBuffers(1, &vertex_buffer_object_position_);
        glDeleteBuffers(1, &vertex_buffer_object_index_);
        glDeleteVertexArrays(1, &vertex_array_id_);
        glDeleteProgram(program_id_);
    }

    void Draw(const mat4 &VIEW,
              const mat4 &PROJECTION) {
        glUseProgram(program_id_);

        mat4 skyboxMVP = PROJECTION * VIEW;//mat4(mat3(VIEW));

        float time = glfwGetTime();
        float theta = 0.05 * time - 0.5;

        vec3 sunPos = sunOrbitCenter + radius * cos(theta) * sunOrbitXAxis + radius * sin(theta) * sunOrbitYAxis;
        computeSkyColors(sunPos);

        light->setPos(sunPos);
        glUniformMatrix4fv(MVPId, 1, GL_FALSE, value_ptr(skyboxMVP));
        glUniform3fv(sunPosId, 1, value_ptr(sunPos));
        glUniform3fv(topSkyColorId, 1, value_ptr(topSkyColor));
        glUniform3fv(bottomSkyColorId, 1, value_ptr(bottomSkyColor));

        // dome
        glBindVertexArray(vertex_array_id_);
        // glDepthMask(GL_FALSE);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

        //glDepthMask(GL_TRUE);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    float getRadius(){
        return radius;
    }

    void computeSkyColors(const vec3& sunPos){

        if(sunPos.y < sunsetBegin){
            float sunSetCoeff = clamp((sunPos.y - nightBegin) / (sunsetBegin - nightBegin), 0.0f, 1.0f);
            bottomSkyColor = mix(nightSkyColor, SUNSETCOL_bottomSky, sunSetCoeff);
            topSkyColor = mix(nightSkyColor, SUNSETCOL_topSky, sunSetCoeff);
            light->setAmbientIntensity(mix(LIGHTCOL_NIGHT, LIGHTCOL_SUNSET, sunSetCoeff));
        }
        else if(sunPos.y > sunsetBegin && sunPos.y < sunsetEnd){

            float sunSetCoeff = (sunPos.y - sunsetBegin) / (sunsetEnd - sunsetBegin);
            sunSetCoeff = Utils::smoothExpTransition(sunSetCoeff);
            bottomSkyColor = mix(SUNSETCOL_bottomSky, mistColor, sunSetCoeff);
            topSkyColor = mix(SUNSETCOL_topSky, blueSkyColor, sunSetCoeff);
            light->setAmbientIntensity(mix(LIGHTCOL_SUNSET, light->getDefaultDiffuseIntensity(), sunSetCoeff));
        } else {
            bottomSkyColor = mistColor;
            topSkyColor = blueSkyColor;
        }
    }
};
