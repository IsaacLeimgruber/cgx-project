#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>
#include "../gridmesh.h"

using namespace glm;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Grass: public GridMesh {

private:
    GLuint vertex_array_id_;        // vertex array object
    GLuint program_id_;             // GLSL shader program ID
    GLuint vertex_buffer_object_;   // memory buffer
    GLuint texture_id_;             // texture ID
    GLuint translation_id_;
    GLuint translationToSceneCenter_id_;
    GLuint VP_id_;          // Model, view, projection matrix ID
    GLuint quadVAO, quadVBO;
    GLuint rows = 20;
    GLuint cols = rows;
    GLuint nBush = rows * cols;


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
        glGenVertexArrays(1, &vertex_array_id_);
        glBindVertexArray(vertex_array_id_);


        mat4* modelMatrices = new mat4[3*nBush];
        vec2* translations = new vec2[3*nBush];
        srand(glfwGetTime()); // initialize random seed

        GLfloat colWidth = 2.f/rows;
        GLfloat rowHeight = 2.f/rows;
        glm::vec3 originOffset = glm::vec3(-1, 0, -1);
        GLfloat scaleRatio = 0.08;

        for (int iBush = 0; iBush < rows; ++iBush) {
            for (int jBush = 0; jBush < cols; ++jBush) {

                GLfloat xBush = colWidth  * jBush + originOffset.x;
                GLfloat yBush = 0 + originOffset.y;
                GLfloat zBush = (rowHeight * iBush + originOffset.z);

                mat4 modelMatrix = translate(IDENTITY_MATRIX, vec3(xBush, yBush, zBush));
                modelMatrix = scale(modelMatrix, vec3(scaleRatio));

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

        // Generate quad VAO
        const GLfloat quadVertices[] = { -0.5f, -0.5f, 0.0f,
                                         +0.5f, -0.5f, 0.0f,
                                         -0.5f, +0.5f, 0.0f,
                                         +0.5f, +0.5f, 0.0f
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
                                                           /*V4*/ 1.0f, 1.0f};

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

            loadHeightMap(heightMap);
            loadGrassMap(grassMap);

            int width;
            int height;
            int nb_component;
            string filename = "grassAlpha.tga";
            // set stb_image to have the same coordinates as OpenGL
            stbi_set_flip_vertically_on_load(1);
            unsigned char* image = stbi_load(filename.c_str(), &width,
                                             &height, &nb_component, 0);

            if(image == nullptr) {
                throw(string("Failed to load texture"));
            }

            glGenTextures(1, &texture_id_);
            glBindTexture(GL_TEXTURE_2D, texture_id_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            if(nb_component == 3) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, image);
            } else if(nb_component == 4) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, image);
            }

            GLuint tex_id = glGetUniformLocation(program_id_, "colorTex");
            glUniform1i(tex_id, 7);
            // cleanup
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(image);
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
        glDeleteVertexArrays(1, &vertex_array_id_);
        glDeleteTextures(1, &texture_id_);
    }

    void Draw(const mat4 &VP = IDENTITY_MATRIX, vec2 translation = vec2(0.f, 0.f), const glm::vec2 &translationToSceneCenter = glm::vec2(0,0)) {
        glUseProgram(program_id_);
        glBindVertexArray(vertex_array_id_);

        // bind textures
        bindHeightMapTexture();
        bindGrassMapTexture();
        glActiveTexture(GL_TEXTURE0 + 7);
        glBindTexture(GL_TEXTURE_2D, texture_id_);

        // setup MVP
        glUniformMatrix4fv(VP_id_, ONE, DONT_TRANSPOSE,
                           glm::value_ptr(VP));
        glUniform2fv(translation_id_, 1, glm::value_ptr(translation));
        glUniform2fv(translationToSceneCenter_id_, 1, glm::value_ptr(translationToSceneCenter));

        // setup MVP
        //glUniformMatrix4fv(view_id_, ONE, DONT_TRANSPOSE,
          //                 glm::value_ptr(IDENTITY_MATRIX));

        // draw
        //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(quadVAO);

        //grass quads must be able to overlap
        glDepthMask(false);
        glDisable(GL_CULL_FACE);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 3*nBush); // 3*amount quads of 4 vertices each
        glEnable(GL_CULL_FACE);
        glDepthMask(true);

        glBindVertexArray(0);
        glUseProgram(0);
    }
};
