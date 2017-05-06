#ifndef PERLIN_TEXTURE_H
#define PERLIN_TEXTURE_H

#include "perlin.h"
#include "../framebuffer.h"

class PerlinTexture {
    Perlin perlin;
    ColorFBO noiseBuffer;
    glm::vec2 position = glm::vec2(0.0f);

public:
    GLuint init() {
        perlin.Init();
        int noiseBuffer_texture_id = noiseBuffer.Init(1024, 1024, GL_RGB32F, GL_RGB, GL_FLOAT, true);

        return noiseBuffer_texture_id;
    }

    void recompute() {
        noiseBuffer.Bind();
        perlin.Draw(position);
        noiseBuffer.Unbind();
    }

    void move(glm::vec2 update) {
        position += update;
    }

    void setPos(glm::vec2 p) {
        position = p;
    }

    void cleanup() {
        perlin.Cleanup();
        noiseBuffer.Cleanup();
    }
};

#endif // PERLIN_TEXTURE_H
