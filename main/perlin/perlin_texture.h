#ifndef PERLIN_TEXTURE_H
#define PERLIN_TEXTURE_H

#include "perlin.h"
#include "../framebuffer.h"

class PerlinTexture {
    Perlin perlin;
    ColorFBO noiseBuffer;
    glm::vec2 position = glm::vec2(0.0f);
    int textureWidth;
    int textureHeight;

public:
    GLuint init(int textureWidth = 1024, int textureHeight = 1024) {
        this->textureWidth = textureWidth;
        this->textureHeight = textureHeight;

        perlin.Init();
        int noiseBuffer_texture_id = noiseBuffer.Init(textureWidth, textureHeight, GL_RGB32F, GL_RGB, GL_FLOAT, true);

        return noiseBuffer_texture_id;
    }

    void recompute() {
        noiseBuffer.Bind();
        perlin.Draw(position);
        noiseBuffer.Unbind();
    }

    void move(glm::vec2 update) {
        position += textureCorrection(update);
    }

    void setPos(glm::vec2 p) {
        position = textureCorrection(p);
    }

    void cleanup() {
        perlin.Cleanup();
        noiseBuffer.Cleanup();
    }

private:
    glm::vec2 textureCorrection(glm::vec2 pos_offset) {
        //return pos_offset;
        return {
            pos_offset.x * (1 - 1.0 / textureWidth),
            pos_offset.y * (1 - 1.0 / textureHeight)
        };
    }
};

#endif // PERLIN_TEXTURE_H
