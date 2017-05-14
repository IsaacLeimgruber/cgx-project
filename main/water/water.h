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
    GLuint offset_id;
    GLuint time_id;
    GLuint timeDebug_id;
    GLuint diffuseMap_id;

    public:
        Water(){

        }
        void Init(GLuint heightMap, GLuint mirrorMap, GLuint shadowMap) {
            // compile the shaders.
            normalProgramIds.program_id = icg_helper::LoadShaders("water_vshader.glsl",
                                                  "water_fshader.glsl",
                                                  "water_tcshader.glsl",
                                                  "water_teshader.glsl");
            debugProgramIds.program_id = icg_helper::LoadShaders("water_vshader_debug.glsl",
                                                  "water_fshader_debug.glsl",
                                                  "water_tcshader_debug.glsl",
                                                  "water_teshader_debug.glsl",
                                                  "water_gshader_debug.glsl");

            if(!normalProgramIds.program_id || !debugProgramIds.program_id) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(normalProgramIds.program_id);
            currentProgramIds = normalProgramIds;

            // vertex coordinates and indices
            genGrid(16);

            // load texture
            loadNormalMap(Utils::loadImage("waterNormalMap2.tga"));
            loadHeightMap(heightMap);
            loadMirrorMap(mirrorMap);
            loadShadowMap(shadowMap);

            diffuseMap_id = Utils::loadImage("waterColorTexture.tga");
            glUniform1i(glGetUniformLocation(normalProgramIds.program_id, "diffuseMap"), 4);

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            setupLocations();
            time_id = glGetUniformLocation(normalProgramIds.program_id, "time");
            offset_id = glGetUniformLocation(normalProgramIds.program_id, "offset");

            glUseProgram(debugProgramIds.program_id);
            timeDebug_id = glGetUniformLocation(debugProgramIds.program_id, "time");

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
                  const glm::mat4 &MV = IDENTITY_MATRIX,
                  const glm::mat4 &NORMALM = IDENTITY_MATRIX,
                  const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
                  const FractionalView &FV = FractionalView(),
                  const glm::vec2 &offset = glm::vec2(0.0f, 0.0f),
                  const glm::vec2 translation = glm::vec2(0, 0)) {

            glUseProgram(normalProgramIds.program_id);
            currentProgramIds = normalProgramIds;

            bindHeightMapTexture();
            glUniformMatrix4fv(normalProgramIds.SHADOWMVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(SHADOWMVP));
            glUniform1f(time_id, glfwGetTime());
            glUniform2fv(offset_id, 1, glm::value_ptr(offset));
            glUniform2fv(currentProgramIds.translation_id, 1, glm::value_ptr(translation));

            if(light != nullptr)
                light->updateProgram(currentProgramIds.program_id);

            activateTextureUnits();
            setupMVP(MVP, MV, NORMALM);
            setupOffset(FV);

            drawFrame();

            if(debug){
                //New rendering on top of the previous one
                glUseProgram(debugProgramIds.program_id);
                currentProgramIds = debugProgramIds;
                glUniform1f(timeDebug_id, glfwGetTime());
                glUniform2fv(currentProgramIds.translation_id, 1, glm::value_ptr(translation));
                setupMVP(MVP, MV, NORMALM);
                setupOffset(FV);

                drawFrame();
            }


            //deactivateTextureUnits();
            glUseProgram(0);
        }

        void activateTextureUnits(){
            GridMesh::activateTextureUnits(true);
            glActiveTexture(GL_TEXTURE0 + 4);
            glBindTexture(GL_TEXTURE_2D, diffuseMap_id);
        }

};
