#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>

struct Light {
        glm::vec3 La = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 Ld = glm::vec3(0.35, 0.35, 0.35);
        glm::vec3 Ls = glm::vec3(0.9f, 0.9f, 0.9f);
        glm::vec3 light_pos = glm::vec3(5.0f, 2.0f, 0.0f);

        // pass light properties to the shader
        void Setup(GLuint program_id) {
            glUseProgram(program_id);

            // given in camera space
            GLuint light_pos_id = glGetUniformLocation(program_id, "light_pos");

            GLuint La_id = glGetUniformLocation(program_id, "La");
            GLuint Ld_id = glGetUniformLocation(program_id, "Ld");
            GLuint Ls_id = glGetUniformLocation(program_id, "Ls");

            glUniform3fv(light_pos_id, ONE, glm::value_ptr(light_pos));
            glUniform3fv(La_id, ONE, glm::value_ptr(La));
            glUniform3fv(Ld_id, ONE, glm::value_ptr(Ld));
            glUniform3fv(Ls_id, ONE, glm::value_ptr(Ls));
        }
};

struct Material {
        glm::vec3 ka = glm::vec3(0.18f, 0.1f, 0.1f);
        glm::vec3 kd = glm::vec3(0.9f, 0.9f, 0.9f);
        glm::vec3 ks = glm::vec3(0.9f, 0.9f, 0.9f);
        float alpha = 100.0;
        // pass material properties to the shaders
        void Setup(GLuint program_id) {
            glUseProgram(program_id);

            GLuint ka_id = glGetUniformLocation(program_id, "ka");
            GLuint kd_id = glGetUniformLocation(program_id, "kd");
            GLuint ks_id = glGetUniformLocation(program_id, "ks");
            GLuint alpha_id = glGetUniformLocation(program_id, "alpha");

            glUniform3fv(ka_id, ONE, glm::value_ptr(ka));
            glUniform3fv(kd_id, ONE, glm::value_ptr(kd));
            glUniform3fv(ks_id, ONE, glm::value_ptr(ks));
            glUniform1f(alpha_id, alpha);
        }
};

class Grid : public Material, public Light{

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;                     // GLSL shader program ID
        GLuint colorTexture_id_;                     // texture ID
        GLuint normalTexture_id_;
        GLuint num_indices_;                    // number of vertices to render
        GLuint M_id_;                           // model matrix ID
        GLuint V_id_;                           // view matrix ID
        GLuint P_id_;                           // projection matrix ID
        GLuint N_id_;
        GLuint zoom_id_;
        GLuint offset_id_;
        GLfloat zoom = 1;
        bool wireframeDebugEnabled = false;

        glm::vec2 offset = glm::vec2(0,0);

    public:
        void Init(GLuint colorTexture, GLuint normalTexture) {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("grid_vshader.glsl",
                                                  "grid_fshader.glsl",
                                                  "grid_tcshader.glsl",
                                                  "grid_teshader.glsl");
            if(!program_id_) {
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
                this->colorTexture_id_ = colorTexture;
                GLuint heightMapLocation = glGetUniformLocation(program_id_, "heightMap");
                glUniform1i(heightMapLocation, 0);

                glBindTexture(GL_TEXTURE_2D, colorTexture_id_);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                glBindTexture(GL_TEXTURE_2D, 0);

                // load/Assign normal map
                this->normalTexture_id_ = normalTexture;
                GLuint normalMapLocation = glGetUniformLocation(program_id_, "normalMap");
                glUniform1i(normalMapLocation, 1);

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

            zoom_id_ = glGetUniformLocation(program_id_, "zoom");
            offset_id_ = glGetUniformLocation(program_id_, "zoomOffset");

            // Setup material and lighting
            Material::Setup(program_id_);
            Light::Setup(program_id_);

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void toggleWireframeMode(){
            wireframeDebugEnabled = !wireframeDebugEnabled;
        }

        void updateZoomFactor(float z){
            zoom = glm::max(glm::min(4.0f, zoom + z), 0.1f);
            std::cout << "Zoom factor: " << zoom << std::endl;
        }

        void updateOffset(glm::vec2 v){
            offset += v;
            std::cout << "Offset value: (" << offset.x <<", " << offset.y << ")" << std::endl;
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteBuffers(1, &vertex_buffer_object_index_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(program_id_);
            glDeleteTextures(1, &colorTexture_id_);
            glDeleteTextures(1, &normalTexture_id_);
        }

        void Draw(const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX) {

            //glm::mat4 normalMatrix = inverse(transpose(view * model));

            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, colorTexture_id_);

            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, normalTexture_id_);

            // setup MVP
            glUniformMatrix4fv(M_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(model));
            glUniformMatrix4fv(V_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(view));
            glUniformMatrix4fv(P_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(projection));
            //glUniformMatrix4fv(N_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(normalMatrix));

            // setup zoom and offset, ie. what part of the perlin noise we are sampling
            glUniform1f(zoom_id_, zoom);
            glUniform2fv(offset_id_, 1, glm::value_ptr(offset));

            glPolygonMode(GL_FRONT_AND_BACK, (wireframeDebugEnabled) ? GL_LINE : GL_FILL);

            glDrawElements(GL_PATCHES, num_indices_, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
