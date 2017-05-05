#pragma once
#include "icg_helper.h"
#include "../light/light.h"
#include "../utils.h"

class SkyDome{


private:
    GLuint vertex_array_id_;        // vertex array object
    GLuint program_id_;             // GLSL shader program ID
    GLuint vertex_buffer_object_position_;  // memory buffer for positions
    GLuint vertex_buffer_object_index_;     // memory buffer for indices
    GLuint cubemapTexture;
    Light* light;
    const float radius = 1.0f;
    const int rings = 12;
    const int sectors = 12;
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

    void Draw(const glm::mat4 &VIEW,
              const glm::mat4 &PROJECTION) {
        glUseProgram(program_id_);

        glm::mat4 skyboxMVP = PROJECTION * VIEW;//glm::mat4(glm::mat3(VIEW));

        glUniformMatrix4fv(glGetUniformLocation(program_id_, "MVP"), 1, GL_FALSE, glm::value_ptr(skyboxMVP));


        // skybox cube
        glBindVertexArray(vertex_array_id_);
        // glDepthMask(GL_FALSE);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

        //glDepthMask(GL_TRUE);

        glBindVertexArray(0);
        glUseProgram(0);
    }


};
