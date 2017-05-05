#pragma once
#include "icg_helper.h"
#include "../light/light.h"
#include "../utils.h"

class Skybox{

using namespace Utils;

private:
    GLuint vertex_array_id_;        // vertex array object
    GLuint program_id_;             // GLSL shader program ID
    GLuint vertex_buffer_object_;   // memory buffer
    GLuint texture_id_;             // texture ID
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
    Light* light;
    const float hVertsNumber = 8;
    const float vVertsNumber = 8;
    const float domeEndAngle = PI/2.0f;

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

        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;

        float hSpacing = PI2/hVertsNumber;
        float vSpacing = PI/vVertsNumber;

        // generate vertices
        for(float theta = 0.0f; theta < PI2; theta++){
            for(float phi = 0; phi > domeEndAngle; phi++){
                vertices.push_back(-1.0f + i*spacing); vertices.push_back(-1.0f + j*spacing);
            }
        }

        // and indices.
        ffor(float theta = 0.0f; theta < PI2; theta++){
            for(float phi = 0; phi > domeEndAngle; phi++){
                vertices.push_back(-1.0f + i*spacing); vertices.push_back(-1.0f + j*spacing);
            }
        }

        num_indices_ = indices.size();

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
        glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE,
                              ZERO_STRIDE, ZERO_BUFFER_OFFSET);

    }

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
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteProgram(program_id_);
    glDeleteVertexArrays(1, &skyboxVAO);
}

void Draw(const glm::mat4 &VIEW,
          const glm::mat4 &PROJECTION) {
    glUseProgram(program_id_);
    glBindVertexArray(skyboxVAO);

    glm::mat4 skyboxMVP = PROJECTION * glm::mat4(glm::mat3(VIEW));

    glUniformMatrix4fv(glGetUniformLocation(program_id_, "MVP"), 1, GL_FALSE, glm::value_ptr(skyboxMVP));


    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0 + 5);
    glDepthMask(GL_FALSE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
    glUseProgram(0);
}


};
