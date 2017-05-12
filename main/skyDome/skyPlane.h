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

class SkyPlane: public ILightable{

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint vertex_buffer_object_tex_;
        GLuint texture_id_;             // texture ID
        GLuint depthTexture_id_;
        GLuint numIndices;
        GLuint MVP_id, time_id, raymarchDir_id;
        Light* light;
        float planeCurvature = 1.0f;

        float PI = 3.14159265359f;
        float PIovr2 = PI * 0.5f;
        float PI2 = 2.0f * PI ;

    public:
        void Init(GLuint texture, const vec2& stretchCoeff) {

            // compile the shaders
            program_id_ = icg_helper::LoadShaders("skyPlane_vshader.glsl",
                                                  "skyPlane_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex Array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {
                int grid_dim = 16;

                std::vector<GLfloat> vertices;
                std::vector<GLuint> indices;

                float spacing = 2.f/(grid_dim - 1.0f);

                /*
                 * grid is 2 units wide, should contain 100 points -> spacing is 2/100
                 *
                 */

                for(int j = 0; j < grid_dim; j++){
                    for(int i = 0; i < grid_dim; i++){
                        float x = -1.0 + i * spacing;
                        float y = -1.0 + j * spacing;
                        vertices.push_back(x);
                        vertices.push_back(0.1 * (-(x * x) - (y * y) + 2.0));
                        vertices.push_back(y);
                    }
                }

                // and indices.
                for(int j = 0; j < grid_dim-1; j++){
                    for(int i = 0; i < grid_dim-1; i++){
                        //First triangle
                        indices.push_back((j*grid_dim) + (i));
                        indices.push_back(((j)*grid_dim) + (i+1));
                        indices.push_back(((j+1)*grid_dim) + (i+1));
                        indices.push_back((j*grid_dim) + (i));
                        indices.push_back(((j+1)*grid_dim) + (i+1));
                        indices.push_back(((j+1)*grid_dim) + (i));
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
                GLuint loc_position = glGetAttribLocation(program_id_, "vpoint");
                glEnableVertexAttribArray(loc_position);
                glVertexAttribPointer(loc_position, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // load/Assign texture
            this->texture_id_ = Utils::loadImage("cloud1024.tga");
            glBindTexture(GL_TEXTURE_2D, texture_id_);
            GLuint tex_id = glGetUniformLocation(program_id_, "colorTex");
            glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

            MVP_id = glGetUniformLocation(program_id_, "MVP");
            time_id = glGetUniformLocation(program_id_, "time");
            raymarchDir_id = glGetUniformLocation(program_id_, "raymarchDirection");

            glUniform2fv(glGetUniformLocation(program_id_, "stretchCoeff"), 1, value_ptr(stretchCoeff));

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
            glDeleteProgram(program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteTextures(1, &texture_id_);
        }

        void Draw(const glm::mat4& MVP, const glm::vec3& sunPos) {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            glUniformMatrix4fv(MVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));

            glUniform3fv(raymarchDir_id, 1, value_ptr(normalize(sunPos)));
            glUniform1f(time_id, glfwGetTime());

            light->updateProgram(program_id_);

            // bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_id_);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, depthTexture_id_);

            // draw
            glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
