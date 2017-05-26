#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "light/light.h"
#include "light/lightable.h"
#include "material/material.h"
#include "camera/fractionalview.h"

struct ProgramIds{
    GLuint program_id;
    GLuint MVP_id, MV_id, NORMALM_id, SHADOWMVP_id;
    GLuint zoom_id, zoomOffset_id, translation_id;
    GLuint heightMap_id, mirrorMap_id;
    GLuint grassMap_id;
    GLuint alpha_id;
    GLuint translationToSceneCenter_id;
};

class GridMesh: public ILightable{

    protected:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint heightMapTexture_id_;            // texture ID
        GLuint grassMapTexture_id_;
        GLuint normalTexture_id_;
        GLuint shadowTexture_id_;
        GLuint mirrorTexture_id_;

        //IDs needed in the draw call
        ProgramIds currentProgramIds, normalProgramIds, shadowProgramIds, debugProgramIds;

        GLuint num_indices_;
        int firstCorner;
        Light* light;
        Material material;
        int gridDimensions;
        bool debug;
        bool wireframeDebugEnabled;

    public:
        GridMesh(int firstCorner = 0)
            : light{nullptr}
            , material{Material()}
            , debug{false}
            , wireframeDebugEnabled{false}
            , firstCorner{firstCorner}
        {}

        void genGrid(int grid_dim){
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            std::vector<GLfloat> vertices;
            std::vector<GLuint> indices;

            float spacing = 2.f/(grid_dim - 1.0f);

            /*
             * grid is 2 units wide, should contain 100 points -> spacing is 2/100
             *
             */

            for(int j = 0; j < grid_dim; j++){
                for(int i = 0; i < grid_dim; i++){
                    vertices.push_back(-1.0f + i*spacing);
                    vertices.push_back(-1.0f + j*spacing);
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
            GLuint loc_position = glGetAttribLocation(normalProgramIds.program_id, "gridPos");
            glEnableVertexAttribArray(loc_position);
            glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE,
                                  ZERO_STRIDE, ZERO_BUFFER_OFFSET);

        }

        void setupLocations(){
            for (auto pProgramIds : {&shadowProgramIds, &debugProgramIds, &normalProgramIds}) {
                auto& programIds = *pProgramIds;
                glUseProgram(programIds.program_id);
                programIds.MVP_id = glGetUniformLocation(programIds.program_id, "MVP");
                programIds.MV_id = glGetUniformLocation(programIds.program_id, "MV");
                programIds.NORMALM_id = glGetUniformLocation(programIds.program_id, "NORMALM");
                programIds.SHADOWMVP_id = glGetUniformLocation(programIds.program_id, "SHADOWMVP");
                programIds.zoom_id = glGetUniformLocation(programIds.program_id, "zoom");
                programIds.zoomOffset_id = glGetUniformLocation(programIds.program_id, "zoomOffset");
                programIds.translation_id = glGetUniformLocation(programIds.program_id, "translation");
                programIds.heightMap_id = glGetUniformLocation(programIds.program_id, "heightMap");
                programIds.grassMap_id = glGetUniformLocation(programIds.program_id, "grassMap");
                programIds.alpha_id = glGetUniformLocation(programIds.program_id, "alpha");
                programIds.translationToSceneCenter_id = glGetUniformLocation(programIds.program_id,
                                                                              "translationToSceneCenter");
            }

            //normapProgramIds must be used last, or use: glUseProgram(normalProgramIds.program_id) here
        }

        void useLight(Light* l){
            this->light = l;
            light->registerProgram(normalProgramIds.program_id);
            light->registerProgram(debugProgramIds.program_id);
            glUseProgram(normalProgramIds.program_id);
        }

        void useMaterial(Material m){
            this->material = m;
            material.Setup(normalProgramIds.program_id);
            glUseProgram(debugProgramIds.program_id);
                material.Setup(debugProgramIds.program_id);
            glUseProgram(normalProgramIds.program_id);
        }

        void useShadowMap(GLuint id){
            this->shadowTexture_id_ = id;
        }

        void loadHeightMap(GLuint heightMap){
            this->heightMapTexture_id_ = heightMap;

            for(auto pProgramIds : {&debugProgramIds, &normalProgramIds}) {
                glUseProgram(pProgramIds->program_id);
                glUniform1i(pProgramIds->heightMap_id, 0);
            }
        }

        void loadGrassMap(GLuint grassMap){
            this->grassMapTexture_id_ = grassMap;

            for(auto pProgramIds : {&debugProgramIds, &normalProgramIds}) {
                glUseProgram(pProgramIds->program_id);
                glUniform1i(pProgramIds->grassMap_id, 4);
            }
        }

        void useHeightMap(GLuint heightMap) {
            this->heightMapTexture_id_ = heightMap;
        }

        void useGrassMap(GLuint grassMap){
            this->grassMapTexture_id_ = grassMap;
        }

        void loadNormalMap(GLuint normalMap){
            this->normalTexture_id_ = normalMap;
            GLuint normalMapLocation = glGetUniformLocation(normalProgramIds.program_id, "normalMap");
            glUniform1i(normalMapLocation, 1);

            glUseProgram(debugProgramIds.program_id);
                normalMapLocation = glGetUniformLocation(debugProgramIds.program_id, "normalMap");
                glUniform1i(normalMapLocation, 1);
            glUseProgram(normalProgramIds.program_id);
        }

        void loadShadowMap(GLuint shadowMap){
            this->shadowTexture_id_ = shadowMap;
            GLuint shadowMapLocation = glGetUniformLocation(normalProgramIds.program_id, "shadowMap");
            glUniform1i(shadowMapLocation, 2);
        }

        void loadMirrorMap(GLuint mirrorMap){
            this->mirrorTexture_id_ = mirrorMap;
            GLuint mirrorMapLocation = glGetUniformLocation(normalProgramIds.program_id, "mirrorMap");
            glUniform1i(mirrorMapLocation, 3);

            glUseProgram(debugProgramIds.program_id);
                mirrorMapLocation = glGetUniformLocation(debugProgramIds.program_id, "mirrorMap");
                glUniform1i(mirrorMapLocation, 3);
            glUseProgram(normalProgramIds.program_id);
        }

        void toggleDebugMode(){
            debug = !debug;
        }

        void toggleWireFrame(){
            wireframeDebugEnabled = !wireframeDebugEnabled;
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteBuffers(1, &vertex_buffer_object_index_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(normalProgramIds.program_id);
            glDeleteProgram(shadowProgramIds.program_id);
            glDeleteProgram(debugProgramIds.program_id);
            glDeleteTextures(1, &heightMapTexture_id_);
            glDeleteTextures(1, &grassMapTexture_id_);
            glDeleteTextures(1, &normalTexture_id_);
            glDeleteTextures(1, &mirrorTexture_id_);
            glDeleteTextures(1, &shadowTexture_id_);
        }


        void drawFrame(){
            glBindVertexArray(vertex_array_id_);
            glPolygonMode(GL_FRONT_AND_BACK, (wireframeDebugEnabled) ? GL_LINE : GL_FILL);
            glDrawElements(GL_PATCHES, num_indices_, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        void setupMVP(const glm::mat4 &MVP,
                      const glm::mat4 &MV,
                      const glm::mat4 &NORMALM){

            // setup MVP
            glUniformMatrix4fv(currentProgramIds.MVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));
            glUniformMatrix4fv(currentProgramIds.MV_id, ONE, DONT_TRANSPOSE, glm::value_ptr(MV));
            glUniformMatrix4fv(currentProgramIds.NORMALM_id, ONE, DONT_TRANSPOSE, glm::value_ptr(NORMALM));
        }

        void setupOffset(const FractionalView& FV){
            // setup zoom and offset, ie. what part of the perlin noise we are sampling
            glUniform1f(currentProgramIds.zoom_id, FV.zoom);
            glUniform2fv(currentProgramIds.zoomOffset_id, 1, glm::value_ptr(FV.zoomOffset));
        }

        void activateTextureUnits(bool normalTexture = true){
            bindHeightMapTexture();
            bindGrassMapTexture();
            if (normalTexture) {
                bindNormalMapTexture();
            }
            bindShadowTexture();
            bindMirrorTexture();
        }

        void bindHeightMapTexture() {
            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, heightMapTexture_id_);
        }

        void bindNormalMapTexture() {
            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, normalTexture_id_);
        }

        void bindShadowTexture() {
            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, shadowTexture_id_);
        }

        void bindMirrorTexture() {
            glActiveTexture(GL_TEXTURE0 + 3);
            glBindTexture(GL_TEXTURE_2D, mirrorTexture_id_);

        }

        void bindGrassMapTexture() {
            glActiveTexture(GL_TEXTURE0 + 4);
            glBindTexture(GL_TEXTURE_2D, grassMapTexture_id_);
        }

        void deactivateTextureUnits() {
            for (int i = 0; i < 5; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
};
