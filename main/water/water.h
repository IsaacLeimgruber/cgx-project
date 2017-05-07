#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../gridmesh.h"
#include "../light/light.h"
#include "../material/material.h"
#include "../camera/fractionalview.h"
#include "../utils.h"

class Water: public GridMesh{

    private:

    GLuint time_id;
    GLuint timeDebug_id;
    GLuint translationId;
    GLuint translationDebugId;

    public:
        Water(){

        }
        void Init(GLuint heightMap, GLuint mirrorMap, GLuint shadowMap) {
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
            current_program_id_ = program_id_;

            // vertex coordinates and indices
            genGrid(16);

            loadNormalMap(Utils::loadImage("waterNormalMap2.tga"));


            // load texture
            loadHeightMap(heightMap);
            loadMirrorMap(mirrorMap);
            loadShadowMap(shadowMap);

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            setupLocations();
            time_id = glGetUniformLocation(program_id_, "time");
            translationId = glGetUniformLocation(program_id_, "translation");


            glUseProgram(debug_program_id_);
            timeDebug_id = glGetUniformLocation(debug_program_id_, "time");
            translationDebugId = glGetUniformLocation(debug_program_id_, "translation");

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
                  const glm::mat4 &MV = IDENTITY_MATRIX,
                  const glm::mat4 &NORMALM = IDENTITY_MATRIX,
                  const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
                  const FractionalView &FV = FractionalView(),
                  const glm::vec2 translation = glm::vec2(0, 0)) {

            glUseProgram(program_id_);
            current_program_id_ = program_id_;
            currentProgramIds = normalProgramIds;

            glUniformMatrix4fv(normalProgramIds.SHADOWMVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(SHADOWMVP));
            glUniform1f(time_id, glfwGetTime());
            glUniform2fv(translationId, 1, glm::value_ptr(translation));

            if(light != nullptr)
                light->updatePosUniform(current_program_id_);

            activateTextureUnits();
            setupMVP(MVP, MV, NORMALM);
            setupOffset(FV);

            drawFrame();

            if(debug){
                //New rendering on top of the previous one
                glUseProgram(debug_program_id_);
                current_program_id_ = debug_program_id_;
                currentProgramIds = debugProgramIds;
                glUniform1f(timeDebug_id, glfwGetTime());
                glUniform2fv(translationDebugId, 1, glm::value_ptr(translation));
                setupMVP(MVP, MV, NORMALM);
                setupOffset(FV);

                drawFrame();
            }


            //deactivateTextureUnits();
            glUseProgram(0);
        }

};
