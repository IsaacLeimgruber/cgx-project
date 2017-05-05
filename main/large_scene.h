#ifndef LSCENE_H
#define LSCENE_H

#include <array>
#include "scene.h"

class LargeScene {
    static constexpr int NROW = 1;
    static constexpr int NCOL = 1;
    template <class T> using Row = std::array<T, NCOL>;
    template <class T> using Matrix = std::array<Row<T>, NROW>;

    Matrix<Scene> scenes;
public:

    template <class Lambda>
    void foreach_in_matrix(Lambda&& fun) {
        for(int iRow = 0; iRow < NROW; ++iRow)
            for(int jCol = 0; jCol < NCOL; ++jCol)
                fun(scenes[iRow][jCol]);
    }

    void initPerlin() {
        foreach_in_matrix([](Scene& scene){
            scene.initPerlin();
        });

    }

    int init(int shadowBuffer_texture_id, int reflectionBuffer_texture_id, Light* light) {
        foreach_in_matrix([&](Scene& scene){
            scene.init(shadowBuffer_texture_id, reflectionBuffer_texture_id, light);
        });
    }

    void draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
              const glm::mat4 &MV = IDENTITY_MATRIX,
              const glm::mat4 &NORMALM = IDENTITY_MATRIX,
              const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
              const FractionalView &FV = FractionalView(),
              bool mirrorPass = false,
              bool shadowPass = false)
    {
        foreach_in_matrix([&](Scene& scene){
            scene.draw(MVP, MV, NORMALM, SHADOWMVP, FV, mirrorPass, shadowPass);
        });
    }

    void moveNoise(const glm::vec2& update) {
        foreach_in_matrix([&update](Scene& scene){
            scene.moveNoise(update);
        });
    }

    void toggleWireFrame() {
        foreach_in_matrix([](Scene& scene){
            scene.toggleWireFrame();
        });
    }

    void toggleDebugMode() {
        foreach_in_matrix([](Scene& scene){
            scene.toggleDebugMode();
        });
    }

    void cleanup() {
        foreach_in_matrix([](Scene& scene){
            scene.cleanup();
        });
    }
};

#endif // SCENE_H
