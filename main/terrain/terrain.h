#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../gridmesh.h"
#include "../light/light.h"
#include "../material/material.h"
#include "../camera/fractionalview.h"

class Grid: public GridMesh{

    private:

    public:
        Grid(){

        }

        void Init(GLuint heightMap, GLuint normalMap, GLuint shadowMap) {
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
                cin.get();
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex coordinates and indices
            genGrid(16);

            // load texture
            loadHeightMap(heightMap);
            loadNormalMap(normalMap);
            loadShadowMap(shadowMap);

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

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
                  bool shadowPass = false) {

            current_program_id_= (shadowPass) ? shadow_program_id_ : program_id_;
            glUseProgram(current_program_id_);

            glUniformMatrix4fv(glGetUniformLocation(current_program_id_, "SHADOWMVP"), ONE, DONT_TRANSPOSE, glm::value_ptr(SHADOWMVP));

            activateTextureUnits();

            //update light
            if(light != nullptr)
                light->updatePosUniform(current_program_id_);

            // if mirror pass is enabled then we cull underwater fragments
            glUniform1i(glGetUniformLocation(current_program_id_, "mirrorPass"), mirrorPass);

            setupMVP(MVP, MV, NORMALM);
            setupOffset(FV);

            drawFrame();

            if(debug){
                //New rendering on top of the previous one
                glUseProgram(debug_program_id_);
                current_program_id_ = debug_program_id_;

                if(light != nullptr)
                    light->updatePosUniform(current_program_id_);

                setupMVP(MVP, MV, NORMALM);
                setupOffset(FV);

                // if mirror pass is enabled then we cull underwater fragments
                glUniform1i(glGetUniformLocation(current_program_id_, "mirrorPass"), mirrorPass);

                drawFrame();
            }

            deactivateTextureUnits();
            glUseProgram(0);
        }
};
