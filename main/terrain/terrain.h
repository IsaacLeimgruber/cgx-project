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
    GLuint grassTextureId, grassTextureBisId, rockTextureId, sandTextureId, snowTextureId;
    GLuint translationId, translationDebugId;

    public:
        Grid(int firstCorner = 0) : GridMesh(firstCorner)
        {}

        void Init(GLuint heightMap, GLuint shadowMap, GLuint grassMap, int fogStop, int fogLength) {
            // compile the shaders.
            normalProgramIds.program_id = icg_helper::LoadShaders("terrain_vshader.glsl",
                                                  "terrain_fshader.glsl",
                                                  "terrain_tcshader.glsl",
                                                  "terrain_teshader.glsl");

            shadowProgramIds.program_id = icg_helper::LoadShaders("terrain_vshader_shadow.glsl",
                                                  "terrain_fshader_shadow.glsl",
                                                  "terrain_tcshader_shadow.glsl",
                                                  "terrain_teshader_shadow.glsl");


            debugProgramIds.program_id = icg_helper::LoadShaders("terrain_vshader_debug.glsl",
                                                  "terrain_fshader_debug.glsl",
                                                  "terrain_tcshader_debug.glsl",
                                                  "terrain_teshader_debug.glsl",
                                                  "terrain_gshader_debug.glsl");
            if(!normalProgramIds.program_id || !shadowProgramIds.program_id || !debugProgramIds.program_id) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(normalProgramIds.program_id);
            glUniform1f(glGetUniformLocation(normalProgramIds.program_id, "threshold_vpoint_World_F"), fogStop - fogLength);
            glUniform1f(glGetUniformLocation(normalProgramIds.program_id, "max_vpoint_World_F"), fogStop);

            // vertex coordinates and indices
            genGrid(4);

            // load texture
            loadHeightMap(heightMap);
            loadGrassMap(grassMap);
            loadShadowMap(shadowMap);

            // load terrain-specific textures
            {
                grassTextureId = Utils::loadImage("grass.tga");
                grassTextureBisId = Utils::loadImage("ground.tga");
                rockTextureId = Utils::loadImage("rock512.tga");
                sandTextureId = Utils::loadImage("sand256.tga");
                snowTextureId = Utils::loadImage("snow512.tga");
                glUniform1i(glGetUniformLocation(normalProgramIds.program_id, "grassTex"), 4);
                glUniform1i(glGetUniformLocation(normalProgramIds.program_id, "grassbisTex"), 5);
                glUniform1i(glGetUniformLocation(normalProgramIds.program_id, "rockTex"), 6);
                glUniform1i(glGetUniformLocation(normalProgramIds.program_id, "sandTex"), 7);
                glUniform1i(glGetUniformLocation(normalProgramIds.program_id, "snowTex"), 8);
            }

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            setupLocations();
            mirrorPassId = glGetUniformLocation(normalProgramIds.program_id, "mirrorPass");

            glUseProgram(debugProgramIds.program_id);
            mirrorPassDebugId = glGetUniformLocation(debugProgramIds.program_id, "mirrorPass");
            glUseProgram(normalProgramIds.program_id);

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
                  const glm::vec2 &translation = glm::vec2(0, 0),
                  const glm::vec2 &translationToSceneCenter = glm::vec2(0,0)) {

            currentProgramIds = (shadowPass) ? shadowProgramIds : normalProgramIds;

            glUseProgram(currentProgramIds.program_id);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDepthFunc(GL_LESS);

            bindHeightMapTexture();
            bindGrassMapTexture();
            glUniformMatrix4fv(currentProgramIds.SHADOWMVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(SHADOWMVP));
            glUniform2fv(currentProgramIds.translation_id, 1, glm::value_ptr(translation));
            glUniform2fv(currentProgramIds.translationToSceneCenter_id, 1, glm::value_ptr(translationToSceneCenter));
            activateTextureUnits();

            //update light
            if(light != nullptr)
                light->updateProgram(currentProgramIds.program_id);

            // if mirror pass is enabled then we cull underwater fragments
            glUniform1i(mirrorPassId, mirrorPass);

            setupMVP(MVP, MV, NORMALM);
            setupOffset(FV);

            drawFrame();

            if(debug){
                //New rendering on top of the previous one
                currentProgramIds = debugProgramIds;
                glUseProgram(currentProgramIds.program_id);

                if(light != nullptr){
                    light->updateProgram(currentProgramIds.program_id);
                }

                setupMVP(MVP, MV, NORMALM);
                setupOffset(FV);

                // if mirror pass is enabled then we cull underwater fragments
                glUniform1i(mirrorPassDebugId, mirrorPass);
                glUniform2fv(currentProgramIds.translation_id, 1, glm::value_ptr(translation));
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
            glBindTexture(GL_TEXTURE_2D, grassTextureBisId);
            glActiveTexture(GL_TEXTURE0 + 6);
            glBindTexture(GL_TEXTURE_2D, rockTextureId);
            glActiveTexture(GL_TEXTURE0 + 7);
            glBindTexture(GL_TEXTURE_2D, sandTextureId);
            glActiveTexture(GL_TEXTURE0 + 8);
            glBindTexture(GL_TEXTURE_2D, snowTextureId);
        }
};
