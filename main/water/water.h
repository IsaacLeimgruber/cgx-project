#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../gridmesh.h"
#include "../light/light.h"
#include "../material/material.h"
#include "../camera/fractionalview.h"

class Water: public GridMesh{

    private:

    GLuint time_id;
    GLuint timeDebug_id;

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

            int width;
            int height;
            int nb_component;
            unsigned char* image = stbi_load("waterNormalMap2.tga", &width,
                                                             &height, &nb_component, 0);

            if(image == nullptr) {
                throw(string("Failed to load texture"));
            }

            glGenTextures(1, &shadowTexture_id_);
            glBindTexture(GL_TEXTURE_2D, shadowTexture_id_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            if(nb_component == 3) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, image);
            } else if(nb_component == 4) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, image);
            }

            loadNormalMap(shadowTexture_id_);

            // cleanup
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(image);

            // load texture
            loadHeightMap(heightMap);
            loadMirrorMap(mirrorMap);
            loadShadowMap(shadowMap);

            //Tesselation configuration
            glPatchParameteri(GL_PATCH_VERTICES, 4);

            setupLocations();
            time_id = glGetUniformLocation(program_id_, "time");
            glUseProgram(debug_program_id_);
            timeDebug_id = glGetUniformLocation(debug_program_id_, "time");

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
            currentProgramIds = normalProgramIds;

            glUniformMatrix4fv(normalProgramIds.SHADOWMVP_id, ONE, DONT_TRANSPOSE, glm::value_ptr(SHADOWMVP));
            glUniform1f(time_id, glfwGetTime());

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
                setupMVP(MVP, MV, NORMALM);
                setupOffset(FV);

                drawFrame();
            }


            //deactivateTextureUnits();
            glUseProgram(0);
        }

};
