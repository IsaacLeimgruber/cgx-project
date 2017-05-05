#ifndef LSCENE_H
#define LSCENE_H

#include <array>
#include "scene.h"

class LargeScene {
    static constexpr int NROW = 5;
    static constexpr int NCOL = 5;
    template <class T> using Row = std::array<T, NCOL>;
    template <class T> using Matrix = std::array<Row<T>, NROW>;

    Matrix<Scene> scenes;
    Matrix<glm::vec2>  translations;

    float gridSize = 2;
    float perlinSize = 1024;
public:

    template <class Lambda>
    void foreach_in_matrix(Lambda&& fun) {
        for(int iRow = 0; iRow < NROW; ++iRow)
            for(int jCol = 0; jCol < NCOL; ++jCol)
                fun(scenes[iRow][jCol]);
    }

    template <class Lambda>
    void foreach_in_matrices(Lambda&& fun) {
        for(int iRow = 0; iRow < NROW; ++iRow)
            for(int jCol = 0; jCol < NCOL; ++jCol)
                fun(scenes[iRow][jCol], translations[iRow][jCol]);
    }

    void initTranslations() {
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                translations[iRow][jCol] = glm::vec2(iRow - NROW / 2, jCol - NCOL / 2);
            }
        }
    }

    void initPerlin() {
        initTranslations();
        foreach_in_matrices([&](Scene& scene, const glm::vec2& translation){
            scene.initPerlin();
            scene.moveNoise(translation);
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
        foreach_in_matrices([&](Scene& scene, const glm::vec2& translation){
            scene.draw(MVP, MV, NORMALM, SHADOWMVP, FV, mirrorPass, shadowPass, gridSize * translation);
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
