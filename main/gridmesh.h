#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "light/light.h"
#include "material/material.h"
#include "camera/fractionalview.h"

class GridMesh{

    protected:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;                     // GLSL shader program ID
        GLuint current_program_id_;
        GLuint shadow_program_id_;
        GLuint debug_program_id_;
        GLuint heightMapTexture_id_;                     // texture ID
        GLuint normalTexture_id_;
        GLuint shadowTexture_id_;
        GLuint mirrorTexture_id_;
        GLuint num_indices_;
        Light* light;
        Material material;
        int gridDimensions;
        bool debug;
        bool wireframeDebugEnabled;

    public:
        GridMesh():light{nullptr}, material{Material()}, debug{false}, wireframeDebugEnabled{false} {

        }

        void Init(){

        }

        void genGrid(int grid_dim){
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            std::vector<GLfloat> vertices;
            std::vector<GLuint> indices;

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

        void useLight(Light* l){
            this->light = l;
            light->Setup(program_id_);
            glUseProgram(debug_program_id_);
                light->Setup(debug_program_id_);
            glUseProgram(program_id_);
        }

        void useMaterial(Material m){
            this->material = m;
            material.Setup(program_id_);
            glUseProgram(debug_program_id_);
                material.Setup(debug_program_id_);
            glUseProgram(program_id_);
        }

        void useShadowMap(GLuint id){
            this->shadowTexture_id_ = id;
        }

        void loadHeightMap(GLuint heightMap){
            this->heightMapTexture_id_ = heightMap;
            GLuint heightMapLocation = glGetUniformLocation(program_id_, "heightMap");
            glUniform1i(heightMapLocation, 0);

            glUseProgram(debug_program_id_);
                heightMapLocation = glGetUniformLocation(debug_program_id_, "heightMap");
                glUniform1i(heightMapLocation, 0);
            glUseProgram(program_id_);
        }

        void loadNormalMap(GLuint normalMap){
            this->normalTexture_id_ = normalMap;
            GLuint normalMapLocation = glGetUniformLocation(program_id_, "normalMap");
            glUniform1i(normalMapLocation, 1);

            glUseProgram(debug_program_id_);
                normalMapLocation = glGetUniformLocation(debug_program_id_, "normalMap");
                glUniform1i(normalMapLocation, 1);
            glUseProgram(program_id_);
        }

        void loadShadowMap(GLuint shadowMap){
            this->shadowTexture_id_ = shadowMap;
            GLuint shadowMapLocation = glGetUniformLocation(program_id_, "shadowMap");
            glUniform1i(shadowMapLocation, 2);
        }

        void loadMirrorMap(GLuint mirrorMap){
            this->mirrorTexture_id_ = mirrorMap;
            GLuint mirrorMapLocation = glGetUniformLocation(program_id_, "mirrorMap");
            glUniform1i(mirrorMapLocation, 3);

            glUseProgram(debug_program_id_);
                mirrorMapLocation = glGetUniformLocation(debug_program_id_, "mirrorMap");
                glUniform1i(mirrorMapLocation, 3);
            glUseProgram(program_id_);
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
            glDeleteProgram(shadow_program_id_);
            glDeleteProgram(debug_program_id_);
            glDeleteTextures(1, &heightMapTexture_id_);
            glDeleteTextures(1, &normalTexture_id_);
            glDeleteTextures(1, &mirrorTexture_id_);
        }

        void Draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
                  const glm::mat4 &MV = IDENTITY_MATRIX,
                  const glm::mat4 &NORMALM = IDENTITY_MATRIX,
                  const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
                  const FractionalView &FV = FractionalView(),
                  bool mirrorPass = false,
                  bool shadowPass = false) {

        }

        void drawFrame(){
            glBindVertexArray(vertex_array_id_);
            glDrawElements(GL_PATCHES, num_indices_, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        void setupMVP(const glm::mat4 &MVP,
                      const glm::mat4 &MV,
                      const glm::mat4 &NORMALM){
            // setup MVP
            glUniformMatrix4fv(glGetUniformLocation(current_program_id_, "MVP"), ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(glGetUniformLocation(current_program_id_, "MV"), ONE, DONT_TRANSPOSE, glm::value_ptr(MV));
            glUniformMatrix4fv(glGetUniformLocation(current_program_id_, "NORMALM"), ONE, DONT_TRANSPOSE, glm::value_ptr(NORMALM));
        }

        void setupOffset(const FractionalView FV){
            // setup zoom and offset, ie. what part of the perlin noise we are sampling
            glUniform1f(glGetUniformLocation(current_program_id_, "zoom"), FV.zoom);
            glUniform2fv(glGetUniformLocation(current_program_id_, "zoomOffset"), 1, glm::value_ptr(FV.zoomOffset));
        }

        void activateTextureUnits(){
            // bind textures
            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, heightMapTexture_id_);

            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, normalTexture_id_);

            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, shadowTexture_id_);

            glActiveTexture(GL_TEXTURE0 + 3);
            glBindTexture(GL_TEXTURE_2D, mirrorTexture_id_);
        }

        void deactivateTextureUnits(){
            // unbind textures
            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, 0);

            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, 0);

            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, 0);

            glActiveTexture(GL_TEXTURE0 + 3);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
};
