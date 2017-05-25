#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../gridmesh.h"
#include "../utils.h"

using namespace glm;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Grass: public GridMesh {

private:

    GLuint program_id_;             // GLSL shader program ID
    GLuint vertex_buffer_object_;   // memory buffer
    GLuint grassAlpha_id_;             // texture ID
    GLuint translation_id_;
    GLuint heightMapTexture_id_, grassMapTexture_id_;
    GLuint translationToSceneCenter_id_;
    GLuint VP_id_;          // Model, view, projection matrix ID
    GLuint quadVAO, quadVBO;
    GLuint rows = 5;
    GLuint cols = rows;
    GLuint nBush = rows * cols;
    GLuint grass_tex_location = 1;
    vector<mat4> modelMatrices{3*nBush};
    vector<vec2> translations{3*nBush};
    GLfloat bushScaleRatio = 0.08;


public:

    Grass(){

    }
    void Init(GLuint heightMap, GLuint grassMap) {
        // compile the shaders
        program_id_ = icg_helper::LoadShaders("grass_vshader.glsl",
                                              "grass_fshader.glsl");

        if(!program_id_) {
            exit(EXIT_FAILURE);
        }

        glUseProgram(program_id_);

        translation_id_ = glGetUniformLocation(program_id_, "translation");
        VP_id_ = glGetUniformLocation(program_id_, "VP");
        translationToSceneCenter_id_ = glGetUniformLocation(program_id_, "translationToSceneCenter");
        if (translationToSceneCenter_id_ == 0) {
            cout << "Unable to init translationToSceneCenter_id_" << endl;
            exit(EXIT_FAILURE);
        }
        glUniform1f(glGetUniformLocation(program_id_, "threshold_vpoint_World_F"), 2.0f);//fogStop - fogLength);
        glUniform1f(glGetUniformLocation(program_id_, "max_vpoint_World_F"), 10.0f);//fogStop);

        // vertex coordinates and indices
        genGrid(8);

        // vertex one vertex Array
        glGenVertexArrays(1, &quadVAO);
        glBindVertexArray(quadVAO);



        srand(glfwGetTime()); // initialize random seed

        GLfloat colWidth = 2.f/rows;
        GLfloat rowHeight = 2.f/rows;
        glm::vec3 originOffset = glm::vec3(-1, 0, -1);

        GLfloat offset = 0.5f;
        for (int iBush = 0; iBush < rows; ++iBush) {
            for (int jBush = 0; jBush < cols; ++jBush) {
                //for (int iBush = rows - 1; iBush >= 0; --iBush) {
                //for (int jBush = cols - 1; jBush >= 0; --jBush) {
                GLfloat x_random_offset = (rand() % (GLint)(2 * colWidth * 100)) / 100.0f - colWidth;
                GLfloat xBush = colWidth  * jBush + originOffset.x + x_random_offset;
                GLfloat yBush = 0 + originOffset.y;
                GLfloat z_random_offset = (rand() % (GLint)(2 * colWidth * 100)) / 100.0f - colWidth;
                GLfloat zBush = (rowHeight * iBush + originOffset.z + z_random_offset);

                mat4 modelMatrix = translate(IDENTITY_MATRIX, vec3(xBush, yBush, zBush));
                // modelMatrix = scale(modelMatrix, vec3(scaleRatio));

                int curBushNumber = iBush + jBush * rows;

                /* Each 3-tuple of subsequent instance form a bush, having the same position */
                translations[3*curBushNumber    ] = vec2(xBush, zBush);
                translations[3*curBushNumber + 1] = vec2(xBush, zBush);
                translations[3*curBushNumber + 2] = vec2(xBush, zBush);

                /* Each bush is composed of 3 quad-instances, each rotated by pi/3 */
                GLfloat alpha = M_PI/3.f;
                modelMatrices[3*curBushNumber] = modelMatrix;
                modelMatrices[3*curBushNumber + 1] = rotate(modelMatrix,   alpha, glm::vec3(0.f, 1.f, 0.f));
                modelMatrices[3*curBushNumber + 2] = rotate(modelMatrix, 2*alpha, glm::vec3(0.f, 1.f, 0.f));
            }
        }

        // Instances model Vertex Buffer Object
        GLuint modelVBO;
        glGenBuffers(1, &modelVBO);
        glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
        glBufferData(GL_ARRAY_BUFFER, 3*nBush * sizeof(mat4), &modelMatrices[0], GL_STATIC_DRAW);

        // Instances translations Vertex Buffer Object
        GLuint translationsVBO;
        glGenBuffers(1, &translationsVBO);
        glBindBuffer(GL_ARRAY_BUFFER, translationsVBO);
        glBufferData(GL_ARRAY_BUFFER, 3*nBush * sizeof(vec2), &translations[0], GL_STATIC_DRAW);

        float second_quad_angle = M_PI / 3;
        float third_quad_angle = 2 * M_PI / 3;
        float second_quad_maxx = bushScaleRatio* cos(second_quad_angle);
        float second_quad_maxz = bushScaleRatio * sin(second_quad_angle);

        float third_quad_maxx = bushScaleRatio* cos(third_quad_angle);
        float third_quad_maxz = bushScaleRatio* sin(third_quad_angle);
        // Generate quad VAO
        const GLfloat quadVertices[] = {
            //QUAD 1/3 of the bush
            -bushScaleRatio, -bushScaleRatio, 0.0f,
            +bushScaleRatio, -bushScaleRatio, 0.0f,
            -bushScaleRatio, +bushScaleRatio, 0.0f,

            +bushScaleRatio, +bushScaleRatio, 0.0f,
            -bushScaleRatio, +bushScaleRatio, 0.0f,
            +bushScaleRatio, -bushScaleRatio, 0.0f,

            //QUAD 2/3 of the bush
            -second_quad_maxx, -bushScaleRatio, -second_quad_maxz,
            +second_quad_maxx, -bushScaleRatio, +second_quad_maxz,
            -second_quad_maxx, +bushScaleRatio, -second_quad_maxz,

            +second_quad_maxx, +bushScaleRatio, +second_quad_maxz,
            -second_quad_maxx, +bushScaleRatio, -second_quad_maxz,
            +second_quad_maxx, -bushScaleRatio, +second_quad_maxz,

            //QUAD 3/3 of the bush
            -third_quad_maxx, -bushScaleRatio, -third_quad_maxz,
            +third_quad_maxx, -bushScaleRatio, +third_quad_maxz,
            -third_quad_maxx, +bushScaleRatio, -third_quad_maxz,

            +third_quad_maxx, +bushScaleRatio, +third_quad_maxz,
            -third_quad_maxx, +bushScaleRatio, -third_quad_maxz,
            +third_quad_maxx, -bushScaleRatio, +third_quad_maxz


        };


        // Generate quad VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);

        //generate translations instance data
        glBindBuffer(GL_ARRAY_BUFFER, translationsVBO);
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
        glVertexAttribDivisor(7, 1);

        // Also set instance data
        GLsizei vec4Size = sizeof(vec4);
        glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(vec4Size));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(2 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (GLvoid*)(3 * vec4Size));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);





        // texture coordinates
        {
            const GLfloat vertex_texture_coordinates[] = { /*V1*/ 0.0f, 0.0f,
                                                           /*V2*/ 1.0f, 0.0f,
                                                           /*V3*/ 0.0f, 1.0f,

                                                           /*V4*/ 1.0f, 1.0f,
                                                           /*V3*/ 0.0f, 1.0f,
                                                           /*V2*/ 1.0f, 0.0f,

                                                           /*V1*/ 0.0f, 0.0f,
                                                           /*V2*/ 1.0f, 0.0f,
                                                           /*V3*/ 0.0f, 1.0f,

                                                           /*V4*/ 1.0f, 1.0f,
                                                           /*V3*/ 0.0f, 1.0f,
                                                           /*V2*/ 1.0f, 0.0f,

                                                           /*V1*/ 0.0f, 0.0f,
                                                           /*V2*/ 1.0f, 0.0f,
                                                           /*V3*/ 0.0f, 1.0f,

                                                           /*V4*/ 1.0f, 1.0f,
                                                           /*V3*/ 0.0f, 1.0f,
                                                           /*V2*/ 1.0f, 0.0f
                                                         };

            // buffer
            glGenBuffers(1, &vertex_buffer_object_);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_texture_coordinates),
                         vertex_texture_coordinates, GL_STATIC_DRAW);

            // attribute
            GLuint vertex_texture_coord_id = glGetAttribLocation(program_id_,
                                                                 "vtexcoord");
            glEnableVertexAttribArray(vertex_texture_coord_id);
            glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                  DONT_NORMALIZE, ZERO_STRIDE,
                                  ZERO_BUFFER_OFFSET);
        }

        // load texture
        {
            this->heightMapTexture_id_ = heightMap;
            glUseProgram(program_id_);
            glUniform1i(grassMapTexture_id_, 0);

            this->grassMapTexture_id_ = grassMap;
            glUseProgram(program_id_);
            glUniform1i(grassMapTexture_id_, 4);

            grassAlpha_id_ = Utils::loadImage("grassAlpha.tga");
            int grass_id = glGetUniformLocation(program_id_, "grassAlpha");
            glUniform1i(grass_id, grass_tex_location);

        }

        // to avoid the current object being polluted
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void Cleanup() {
        glBindVertexArray(0);
        glUseProgram(0);
        glDeleteBuffers(1, &vertex_buffer_object_);
        glDeleteProgram(program_id_);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteTextures(1, &grassAlpha_id_);
    }

    static float norm(const vec2 &v){
        return sqrt(v.x*v.x + v.y*v.y);
    }

    static float dist(vec2 point, vec2 reference){
        return norm(point - reference);
    }

    static vec2 tr_value(const mat4 &mat){
        float x = mat[3][0];
        float z = mat[3][2];
        return vec2(x, z);
    }

    void sortInstances(const vec2 &cameraPos){

        //sort translations
        sort(begin(translations),
             end(translations),
             [cameraPos](const vec2& lhs, const vec2& rhs){ return dist(cameraPos, lhs) < dist(cameraPos, rhs); });
        //sort models
        //sort(begin(modelMatrices),
        //    end(modelMatrices),
        //  [cameraPos](const mat4& lhs, const mat4& rhs){
        //return dist(cameraPos, tr_value(lhs)) < dist(cameraPos, tr_value(rhs)); });
    }

    void Draw(const mat4 &VP = IDENTITY_MATRIX, vec2 translation = vec2(0.f, 0.f),
              const glm::vec2 &translationToSceneCenter = glm::vec2(0,0),
              const vec2 cameraPos = vec2(0.f, 0.f)) {
        glUseProgram(program_id_);

        //sort translations and model matrices to draw back to front depending on cameraPosition
        sortInstances(cameraPos);

        // bind textures
        bindHeightMapTexture();
        bindGrassMapTexture();

        // setup MVP
        glUniformMatrix4fv(VP_id_, ONE, DONT_TRANSPOSE,
                           glm::value_ptr(VP));
        glUniform2fv(translation_id_, 1, glm::value_ptr(translation));
        glUniform2fv(translationToSceneCenter_id_, 1, glm::value_ptr(translationToSceneCenter));

        glActiveTexture(GL_TEXTURE0 + grass_tex_location);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0 + grass_tex_location);
        glBindTexture(GL_TEXTURE_2D, grassAlpha_id_);

        glBindVertexArray(quadVAO);

        //grass quads must be able to overlap
        glDepthMask(false);

        //We don't want the alpha part of the texture to occlude other bushes or quads
        //So we activate alpha blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        //We want to see grass from any direction (from the back)
        glDisable(GL_CULL_FACE);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 18, 3*nBush); // 3*amount quads of 4 vertices each
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDepthMask(true);

        glBindVertexArray(0);
        glUseProgram(0);
    }
};
