#pragma once
#include "icg_helper.h"
#include "../light/light.h"

class Skybox{

private:
    GLuint vertex_array_id_;        // vertex array object
    GLuint program_id_;             // GLSL shader program ID
    GLuint vertex_buffer_object_;   // memory buffer
    GLuint texture_id_;             // texture ID
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
    Light* light;

private:
    GLuint loadCubemap(vector<const GLchar*> faces)
    {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glActiveTexture(GL_TEXTURE0 + 5);

        int width,height;
        unsigned char* image;

        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        for(GLuint i = 0; i < faces.size(); i++)
        {
            int nb_component;
            image = stbi_load(faces[i], &width,
                                             &height, &nb_component, 0);

            if(image == nullptr) {
                throw(string("Failed to load texture"));
            }
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
            );
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        return textureID;
    }

public:
    void Init() {

        // compile the shaders
        program_id_ = icg_helper::LoadShaders("skybox_vshader.glsl",
                                              "skybox_fshader.glsl");
        if(!program_id_) {
            exit(EXIT_FAILURE);
        }

        glUseProgram(program_id_);


        // vertex coordinates
        {
            GLfloat skyboxVertices[] = {
                // Positions
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,

                -1.0f,  1.0f, -1.0f,
                 1.0f,  1.0f, -1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f
            };



            // Setup skybox
                glGenVertexArrays(1, &skyboxVAO);
                glGenBuffers(1, &skyboxVBO);

                glBindVertexArray(skyboxVAO);
                glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

                glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
                glBindVertexArray(0);
        }

        //load the cube map
        {
            vector<const char*> faces;

            faces.push_back("right.png");
            faces.push_back("left.png");
            faces.push_back("top.png");
            faces.push_back("bottom.png");
            faces.push_back("back.png");
            faces.push_back("front.png");
            cubemapTexture = loadCubemap(faces);
            glUniform1i(glGetUniformLocation(program_id_, "skybox"), 5);
        }

        // to avoid the current object being polluted
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void useLight(Light* l){
        this->light = l;
        light->registerProgram(program_id_);
    }

    void Cleanup() {
        glBindVertexArray(0);
        glUseProgram(0);
        glDeleteBuffers(1, &skyboxVBO);
        glDeleteProgram(program_id_);
        glDeleteVertexArrays(1, &skyboxVAO);
    }

    void Draw(const glm::mat4 &VIEW,
              const glm::mat4 &PROJECTION) {
        glUseProgram(program_id_);
        glBindVertexArray(skyboxVAO);

        glm::mat4 skyboxMVP = PROJECTION * glm::mat4(glm::mat3(VIEW));

        glUniformMatrix4fv(glGetUniformLocation(program_id_, "MVP"), 1, GL_FALSE, glm::value_ptr(skyboxMVP));


         // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0 + 5);
        glDepthMask(GL_FALSE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthMask(GL_TRUE);

        glBindVertexArray(0);
        glUseProgram(0);
    }


};
