#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../gridmesh.h"
#include "../light/light.h"
#include "../material/material.h"
#include "../camera/fractionalview.h"
#include "../utils.h"

class Grid: public GridMesh{

    private:
    GLuint mirrorPassId;
    GLuint mirrorPassDebugId;
    GLuint grassTextureId, rockTextureId, sandTextureId, snowTextureId;
    GLuint translationId;

    public:
        Grid(){

        }

        void Init(GLuint heightMap, GLuint shadowMap) {
            // compile the shaders.
            program_id_ = icg_helper::LoadShaders("terrain_vshader.glsl",
                                                  "terrain_fshader.glsl",
                                                  "terrain_tcshader.glsl",
                                                  "terrain_teshader.glsl");

            shadow_program_id_ = icg_helper::LoadShaders("terrain_vshader_shadow.glsl",
                                                  "terrain_fshader_shadow.glsl",
                                                  "terrain_tcshader_shadow.glsl",
                                                  "terrain_teshader_shadow.glsl");


            debug_program_id_ = icg_helper::LoadShaders("terrain_vshader_debug.glsl",
                                                  "terrain_fshader_debug.glsl",
                                                  "terrain_tcshader_debug.glsl",
                                                  "terrain_teshader_debug.glsl",
                                                  "terrain_gshader_debug.glsl");
            if(!program_id_ || !shadow_program_id_ || !debug_program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex coordinates and indices
            genGrid(16);

            // load texture
            loadHeightMap(heightMap);
            loadShadowMap(shadowMap);

            // load terrain-specific textures
            {
                grassTextureId = Utils::loadImage("grass512.tga");
                rockTextureId = Utils::loadImage("rock512.tga");
                sandTextureId = Utils::loadImage("sand256.tga");
                snowTextureId = Utils::loadImage("snow512.tga");
                glUniform1i(glGetUniformLocation(program_id_, "grassTex"), 4);
                glUniform1i(glGetUniformLocation(program_id_, "rockTex"), 5);
                glUniform1i(glGetUniformLocation(program_id_, "sandTex"), 6);
                glUniform1i(glGetUniformLocation(program_id_, "snowTex"), 7);
            }

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            setupLocations();
            mirrorPassId = glGetUniformLocation(program_id_, "mirrorPass");
            translationId = glGetUniformLocation(program_id_, "translation");

            glUseProgram(debug_program_id_);
            mirrorPassDebugId = glGetUniformLocation(debug_program_id_, "mirrorPass");
            glUseProgram(program_id_);

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void useShadowMap(GLuint id){
            this->shadowTexture_id_ = id;
        }

        void Draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
                  const glm::mat4 &MV = IDENTITY_MATRIX,
                  const glm::mat4 &NORMALM = IDENTITY_MATRIX,
                  const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
                  const FractionalView &FV = FractionalView(),
                  bool mirrorPass = false,
                  bool shadowPass = false,
                  const glm::vec2 &translation = glm::vec2(0, 0)) {

            current_program_id_= (shadowPass) ? shadow_program_id_ : program_id_;
            currentProgramIds = (shadowPass) ? shadowProgramIds : normalProgramIds;

            glUseProgram(current_program_id_);

            glUniformMatrix4fv(currentProgramIds.SHADOWMVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(SHADOWMVP));
            glUniform2fv(translationId, 1, glm::value_ptr(translation));
            activateTextureUnits();

            //update light
            if(light != nullptr)
                light->updatePosUniform(current_program_id_);

            // if mirror pass is enabled then we cull underwater fragments
            glUniform1i(mirrorPassId, mirrorPass);

            setupMVP(MVP, MV, NORMALM);
            setupOffset(FV);

            drawFrame();

            if(debug){
                //New rendering on top of the previous one
                glUseProgram(debug_program_id_);
                current_program_id_ = debug_program_id_;
                currentProgramIds = debugProgramIds;

                if(light != nullptr){
                    light->updatePosUniform(current_program_id_);
                }

                setupMVP(MVP, MV, NORMALM);
                setupOffset(FV);

                // if mirror pass is enabled then we cull underwater fragments
                glUniform1i(mirrorPassDebugId, mirrorPass);

                drawFrame();
            }

            //deactivateTextureUnits();
            glUseProgram(0);
        }

        void activateTextureUnits(){
            GridMesh::activateTextureUnits(false);
            glActiveTexture(GL_TEXTURE0 + 4);
            glBindTexture(GL_TEXTURE_2D, grassTextureId);
            glActiveTexture(GL_TEXTURE0 + 5);
            glBindTexture(GL_TEXTURE_2D, rockTextureId);
            glActiveTexture(GL_TEXTURE0 + 6);
            glBindTexture(GL_TEXTURE_2D, sandTextureId);
            glActiveTexture(GL_TEXTURE0 + 7);
            glBindTexture(GL_TEXTURE_2D, snowTextureId);
        }
};
