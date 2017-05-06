#ifndef SCENE_H
#define SCENE_H

#include "terrain/terrain.h"
#include "perlin/perlin_texture.h"
#include "water/water.h"

class Scene {
    Grid grid;
    PerlinTexture perlinTexture;
    Water water;
    int noiseBuffer_texture_id;

public:
    void initPerlin() {
        noiseBuffer_texture_id = perlinTexture.init();
    }

    int init(int shadowBuffer_texture_id, int reflectionBuffer_texture_id, Light* light) {
        grid.Init(noiseBuffer_texture_id, shadowBuffer_texture_id);
        grid.useLight(light);
        water.Init(noiseBuffer_texture_id, reflectionBuffer_texture_id, shadowBuffer_texture_id);
        water.useLight(light);
        perlinTexture.recompute();
    }

    void draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
              const glm::mat4 &MV = IDENTITY_MATRIX,
              const glm::mat4 &NORMALM = IDENTITY_MATRIX,
              const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
              const FractionalView &FV = FractionalView(),
              bool mirrorPass = false,
              bool shadowPass = false,
              glm::vec2 tranlation = glm::vec2(0, 0))
    {
        grid.Draw(MVP, MV, NORMALM, SHADOWMVP, FV, mirrorPass, shadowPass, tranlation);
        if (!mirrorPass && !shadowPass) {
            water.Draw(MVP, MV, NORMALM, SHADOWMVP, FV, tranlation);
        }
    }

    void setNoisePos(glm::vec2 pos) {
        perlinTexture.setPos(pos);
        perlinTexture.recompute();
    }

    void moveNoise(glm::vec2 update) {
        perlinTexture.move(std::move(update));
        perlinTexture.recompute();
    }

    void toggleWireFrame() {
        water.toggleWireFrame();
        grid.toggleWireFrame();
    }

    void toggleDebugMode() {
        water.toggleDebugMode();
        grid.toggleDebugMode();
    }

    void cleanup() {
        grid.Cleanup();
        perlinTexture.cleanup();
        water.Cleanup();
    }
};

#endif // SCENE_H
