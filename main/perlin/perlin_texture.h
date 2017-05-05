#ifndef PERLIN_TEXTURE_H
#define PERLIN_TEXTURE_H

#include "perlin.h"
#include "../framebuffer.h"

class PerlinTexture {
    Perlin perlin;
    ColorFBO noiseBuffer;
    FractionalView perlinOffset;

public:
    void init() {
        perlin.Init();
        return noiseBuffer.Init(1024, 1024, GL_RGB32F, GL_RGB, GL_FLOAT, true);
    }
};

#endif // PERLIN_TEXTURE_H
