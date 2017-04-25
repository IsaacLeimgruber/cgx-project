#pragma once
#include <cstdlib>
#include <random>
#include <ctime>
#include "icg_helper.h"

class NormalMap {

private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_;   // memory buffer
        GLuint texture_id_;             // texture ID

        float textureWidth = 1024;
        float textureHeight = 1024;

        int offset_id_;
        glm::vec2 offset = glm::vec2(0,0);

        int scale_id_;
        glm::vec2 scale = glm::vec2(1,1);

public:

        void Init(GLuint texture) {

            // compile the shaders
            program_id_ = icg_helper::LoadShaders("normalmap_vshader.glsl",
                                                  "normalmap_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex Array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {
                const GLfloat vertex_point[] = { /*V1*/ -1.0f, -1.0f, 0.0f,
                                                 /*V2*/ +1.0f, -1.0f, 0.0f,
                                                 /*V3*/ -1.0f, +1.0f, 0.0f,
                                                 /*V4*/ +1.0f, +1.0f, 0.0f};
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_point),
                             vertex_point, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(program_id_, "vpoint");
                glEnableVertexAttribArray(vertex_point_id);
                glVertexAttribPointer(vertex_point_id, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // texture coordinates
            {
                const GLfloat vertex_texture_coordinates[] = { /*V1*/ 0.0f, 0.0f,
                                                               /*V2*/ 1.0f, 0.0f,
                                                               /*V3*/ 0.0f, 1.0f,
                                                               /*V4*/ 1.0f, 1.0f};

                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_texture_coordinates),
                             vertex_texture_coordinates, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(program_id_,
                                                                     "vtexcoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE, ZERO_STRIDE,
                                      ZERO_BUFFER_OFFSET);
            }

            // load texture
            {
                // load/Assign texture
                this->texture_id_ = texture;
                glBindTexture(GL_TEXTURE_2D, texture_id_);
                GLuint tex_id = glGetUniformLocation(program_id_, "heightMap");
                glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                glBindTexture(GL_TEXTURE_2D, 0);

                glUniform1f(glGetUniformLocation(program_id_, "textureWidth"), textureWidth);
                glUniform1f(glGetUniformLocation(program_id_, "textureHeight"), textureHeight);
            }

            scale_id_ = glGetUniformLocation(program_id_, "scale");
            offset_id_ = glGetUniformLocation(program_id_, "offset");

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_);
            glDeleteProgram(program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteTextures(1, &texture_id_);
        }

        void updateScale(glm::vec2 v){
          scale = v;
          //std::cout << "Scale: " << v.x << ", " << v.y << std::endl;
        }

        void updateOffset(glm::vec2 v){
          offset = v;
          //std::cout << "Offset: " << v.x << ", " << v.y << std::endl;
        }

        void Draw(glm::vec2 xyoffset, glm::vec2 xyscale, glm::vec3 translation = glm::vec3(0,0,0)) {
          Draw(xyoffset.x, xyoffset.y, xyscale.x, xyscale.y, translation);
        }

        void Draw(float xoffset = 0, float yoffset = 0, float xscale = 1, float yscale = 1, glm::vec3 translation = glm::vec3(0,0,0)) {
          glUseProgram(program_id_);

          glUniform1f(glGetUniformLocation(program_id_, "xscale"), xscale);
          glUniform1f(glGetUniformLocation(program_id_, "yscale"), yscale);
          glUniform1f(glGetUniformLocation(program_id_, "xoffset"), xoffset);
          glUniform1f(glGetUniformLocation(program_id_, "yoffset"), yoffset);
          glUniform3fv(glGetUniformLocation(program_id_, "translation"), 1, &translation[0]);
          glUniform2fv(scale_id_, 1, glm::value_ptr(scale));
          glUniform2fv(offset_id_, 1, glm::value_ptr(offset));

          glBindVertexArray(vertex_array_id_);
          // bind texture
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, texture_id_);

          // draw
          glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

          glBindVertexArray(0);
          glUseProgram(0);
        }
};
