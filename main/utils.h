#pragma once

#include <glm/gtc/type_ptr.hpp>

class Utils{
public:

    static float smoothExpTransition(float v){
        float x = glm::clamp(v, 0.0f, 1.0f);
        return 1.0f - exp( -pow(2.0f * x , 2));

    }

    static float expDeceleratingTransition(float v){
        float x = glm::clamp(v, 0.0f, 1.0f);
        return 1.0f - exp( -4.0f * x);
    }

    static float expAcceleratingTransition(float v){
        float x = glm::clamp(v, 0.0f, 1.0f);
        return exp( 4.0f * (x - 1.0));
    }

    static GLuint loadImage(const char* filename){
        GLuint texId;

        int width;
        int height;
        int nb_component;
        unsigned char* image = stbi_load(filename, &width, &height, &nb_component, 0);

        if(image == nullptr) {
            throw(string("Failed to load texture"));
        }

        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if(nb_component == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                         GL_RGB, GL_UNSIGNED_BYTE, image);
        } else if(nb_component == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, image);
        }
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(image);
        return texId;
    }
};
