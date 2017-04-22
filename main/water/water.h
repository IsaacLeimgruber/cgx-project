#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../light/light.h"
#include "../material/material.h"

class Water{

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;                     // GLSL shader program ID
        GLuint debug_program_id_;
        GLuint heightMapTexture_id_;            // texture ID
        GLuint mirrorTexture_id_;
        GLuint normalTexture_id_;
        GLuint num_indices_;                    // number of vertices to render
        GLuint M_id_;                           // model matrix ID
        GLuint V_id_;                           // view matrix ID
        GLuint P_id_;                           // projection matrix ID
        GLuint N_id_;
        GLuint time_id_;
        GLuint offset_id_;
        GLuint zoom_id_;
        Light light;
        Material material;
        bool wireframeDebugEnabled;
        bool debug;

    public:
        Water(): light{Light()}, material{Material()}, debug{false},wireframeDebugEnabled{false}{

        }
        void Init(GLuint mirrorTexture, GLuint heightMapTexture, GLuint normalTexture) {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("water_vshader.glsl",
                                                  "water_fshader.glsl",
                                                  "water_tcshader.glsl",
                                                  "water_teshader.glsl");
            debug_program_id_ = icg_helper::LoadShaders("water_vshader_debug.glsl",
                                                  "water_fshader_debug.glsl",
                                                  "water_tcshader_debug.glsl",
                                                  "water_teshader_debug.glsl",
                                                  "water_gshader_debug.glsl");
            if(!program_id_ || !debug_program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates and indices
            {
                std::vector<GLfloat> vertices;
                std::vector<GLuint> indices;

                int grid_dim = 16;
                float spacing = 2.f/(grid_dim - 1.0);

                /*
                 * grid is 2 units wide, should contain 100 points -> spacing is 2/100
                 *
                 */

                for(int j = 0; j < grid_dim; j++){
                    for(int i = 0; i < grid_dim; i++){
                        vertices.push_back(-1.0f + i*spacing); vertices.push_back(-1.0f + j*spacing);
                    }
                }

                // and indices.
                for(int j = 0; j < grid_dim-1; j++){
                    for(int i = 0; i < grid_dim-1; i++){
                        //First triangle
                        indices.push_back((j*grid_dim) + (i));
                        indices.push_back(((j)*grid_dim) + (i+1));
                        indices.push_back(((j+1)*grid_dim) + (i+1));
                        indices.push_back(((j+1)*grid_dim) + (i));
                        //Second triangle
                        //indices.push_back((j*grid_dim) + (i));
                        //indices.push_back(((j+1)*grid_dim) + (i+1));
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
                GLuint loc_position = glGetAttribLocation(program_id_, "gridPos");
                glEnableVertexAttribArray(loc_position);
                glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // load texture
            {
                // load/Assign texture
                this->heightMapTexture_id_ = heightMapTexture;
                GLuint heightMapLocation = glGetUniformLocation(program_id_, "heightMap");
                glUniform1i(heightMapLocation, 0);

                glBindTexture(GL_TEXTURE_2D, heightMapTexture_id_);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                glBindTexture(GL_TEXTURE_2D, 0);

                // load mirror texture
                this->mirrorTexture_id_ = mirrorTexture;
                GLuint mirrorLocation = glGetUniformLocation(program_id_, "mirrorTex");
                glUniform1i(mirrorLocation, 1);

                glBindTexture(GL_TEXTURE_2D, mirrorTexture_id_);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                glBindTexture(GL_TEXTURE_2D, 0);

                // load normal map
                this->normalTexture_id_ = normalTexture;
                GLuint normalMapLocation = glGetUniformLocation(program_id_, "normalMap");
                glUniform1i(normalMapLocation, 2);

                glBindTexture(GL_TEXTURE_2D, normalTexture_id_);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            // other uniforms
            M_id_ = glGetUniformLocation(program_id_, "model");
            V_id_ = glGetUniformLocation(program_id_, "view");
            P_id_ = glGetUniformLocation(program_id_, "projection");
            N_id_ = glGetUniformLocation(program_id_, "normalMatrix");

            time_id_ = glGetUniformLocation(program_id_, "time");
            offset_id_ = glGetUniformLocation(program_id_, "zoomOffset");
            zoom_id_ = glGetUniformLocation(program_id_, "zoom");

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void useLight(Light l){
            this->light = l;
            light.Setup(program_id_);
            glUseProgram(debug_program_id_);
                light.Setup(debug_program_id_);
            glUseProgram(program_id_);
        }

        void useMaterial(Material m){
            this->material = m;
            material.Setup(program_id_);
            glUseProgram(debug_program_id_);
                material.Setup(debug_program_id_);
            glUseProgram(program_id_);
        }

        void toggleWireframeMode(){
            wireframeDebugEnabled = !wireframeDebugEnabled;
        }

        void toggleDebugMode(){
            debug = !debug;
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteBuffers(1, &vertex_buffer_object_index_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(program_id_);
            glDeleteTextures(1, &heightMapTexture_id_);
            glDeleteTextures(1, &mirrorTexture_id_);
        }

        void Draw(const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX,
                  glm::vec2 offset = glm::vec2(0.0f),
                  float zoom = 1.0f) {

            glm::mat4 normalMatrix = inverse(transpose(view * model));

            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, heightMapTexture_id_);

            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, mirrorTexture_id_);

            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, normalTexture_id_);

            // setup MVP
            glUniformMatrix4fv(M_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(model));
            glUniformMatrix4fv(V_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(view));
            glUniformMatrix4fv(P_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(projection));
            glUniformMatrix4fv(N_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(normalMatrix));

            glUniform1f(time_id_, glfwGetTime());
            glUniform1f(zoom_id_, zoom);
            glUniform2fv(offset_id_, 1, glm::value_ptr(offset));

            //glPolygonMode(GL_FRONT_AND_BACK, (wireframeDebugEnabled) ? GL_LINE : GL_FILL);

            glDrawElements(GL_PATCHES, num_indices_, GL_UNSIGNED_INT, 0);

            if(debug){
                glUseProgram(debug_program_id_);
                glBindVertexArray(vertex_array_id_);
                // setup MVP
                glUniformMatrix4fv(glGetUniformLocation(debug_program_id_, "model"), ONE, DONT_TRANSPOSE, glm::value_ptr(model));
                glUniformMatrix4fv(glGetUniformLocation(debug_program_id_, "view"), ONE, DONT_TRANSPOSE, glm::value_ptr(view));
                glUniformMatrix4fv(glGetUniformLocation(debug_program_id_, "projection"), ONE, DONT_TRANSPOSE, glm::value_ptr(projection));
                glUniformMatrix4fv(glGetUniformLocation(debug_program_id_, "normalMatrix"), ONE, DONT_TRANSPOSE, glm::value_ptr(normalMatrix));

                glUniform1f(glGetUniformLocation(debug_program_id_, "time"), glfwGetTime());
                glUniform1f(glGetUniformLocation(debug_program_id_, "zoom"), zoom);
                glUniform2fv(glGetUniformLocation(debug_program_id_, "zoomOffset"), 1, glm::value_ptr(offset));

                glDrawElements(GL_PATCHES, num_indices_, GL_UNSIGNED_INT, 0);
            }
            glBindVertexArray(0);
            glUseProgram(0);
        }
};
