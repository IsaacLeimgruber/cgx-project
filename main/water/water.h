#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../gridmesh.h"
#include "../light/light.h"
#include "../material/material.h"
#include "../camera/fractionalview.h"

class Water: public GridMesh{

    private:

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

            // load texture
            loadHeightMap(heightMap);
            loadMirrorMap(mirrorMap);
            loadShadowMap(shadowMap);

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
                  const glm::mat4 &MV = IDENTITY_MATRIX,
                  const glm::mat4 &NORMALM = IDENTITY_MATRIX,
                  const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
                  const FractionalView &FV = FractionalView()) {

            glUseProgram(program_id_);
            current_program_id_ = program_id_;

            glUniformMatrix4fv(glGetUniformLocation(current_program_id_, "SHADOWMVP"), ONE, DONT_TRANSPOSE, glm::value_ptr(SHADOWMVP));

            glUniform1f(glGetUniformLocation(current_program_id_, "time"), glfwGetTime());
            if(light != nullptr)
                light->updatePosUniform(program_id_);

            activateTextureUnits();
            setupMVP(MVP, MV, NORMALM);
            setupOffset(FV);

            drawFrame();

            if(debug){
                //New rendering on top of the previous one
                glUseProgram(debug_program_id_);
                current_program_id_ = debug_program_id_;
                glUniform1f(glGetUniformLocation(current_program_id_, "time"), glfwGetTime());
                setupMVP(MVP, MV, NORMALM);
                setupOffset(FV);

                drawFrame();
            }

            deactivateTextureUnits();
            glUseProgram(0);
        }
};
