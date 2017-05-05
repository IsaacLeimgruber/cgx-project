#ifndef SCENE_H
#define SCENE_H

#include "terrain/terrain.h"
#include "perlin/perlin.h"

class Scene {
    Grid grid;
    Perlin perlin;
    ColorFBO noiseBuffer;
    Water water;
};

#endif // SCENE_H
