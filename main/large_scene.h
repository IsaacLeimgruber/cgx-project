#ifndef LSCENE_H
#define LSCENE_H

#include <array>
#include <iostream>
#include "scene.h"

/** A LargeScene is an infinite procedural terrain. Internally, it is a circular grid of Scene objects */
class LargeScene {

    /** the dimensions of the large scene's rectangular matrix */
    enum { NROW = 5, NCOL = 5};

    /** the dimension of a small scene as seen per the scene's vertex shader 2 = size([-1;1]) */
    const float gridSize = 2;

    /** the translation is not full because we want a small overlaping, hence a scaling < 1 */
    const float translationScale = .99f;

    template <class T> using Row = std::array<T, NCOL>;
    template <class T> using Matrix = std::array<Row<T>, NROW>;

    /** the matrix i-coordinate of the scene displayed in the bottom left corner */
    int rowStart = 0;

    /** the matrix j-coordinate of the scene displayed in the bottom left corner */
    int colStart = 0;

    /** the noise position of the scene displayed in the bottom left corner */
    glm::vec2 noisePosition {0, 0};

public:
    enum Direction { UP = +1, DOWN = -1 };

    /** initializes the perlin textures */
    void initPerlin() {
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                scene(iRow, jCol).initPerlin();
                scene(iRow, jCol).setNoisePos(noisePosFor(iRow, jCol));
            }
        }
    }

    /** initializes the scenes object */
    void init(int shadowBuffer_texture_id, int reflectionBuffer_texture_id, Light* light) {
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
               scene(iRow, jCol).init(shadowBuffer_texture_id, reflectionBuffer_texture_id, light);
            }
        }
    }

    /** draws every scenes side by side in an ordered manner */
    void draw(const glm::mat4 &MVP = IDENTITY_MATRIX,
              const glm::mat4 &MV = IDENTITY_MATRIX,
              const glm::mat4 &NORMALM = IDENTITY_MATRIX,
              const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
              const FractionalView &FV = FractionalView(),
              bool mirrorPass = false,
              bool shadowPass = false)
    {
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
               scene(iRow, jCol).draw(MVP, MV, NORMALM, SHADOWMVP, FV,
                                      mirrorPass, shadowPass,
                                      gridSize * translation(iRow, jCol));
            }
        }
    }

    /** moves the scenes one column in the given direction, recomputes only obsolete scenes */
    void moveCols(Direction d) {
        int oldColStart = colStart;
        colStart = (colStart - d + NCOL) % NCOL;
        noisePosition.x -= d;

        int col = (d == DOWN) ? oldColStart : colStart;
        for(int iRow = 0; iRow < NROW; ++iRow) {
            scene(iRow, col).setNoisePos(noisePosFor(iRow, col));
        }
    }

    /** moves the scenes one row in the given direction, recomputes only obsolete scenes */
    void moveRows(Direction d) {
        int oldRowStart = rowStart;
        rowStart = (rowStart - d + NROW) % NROW;
        noisePosition.y -= d;

        int row = (d == DOWN) ? oldRowStart : rowStart;
        for(int jCol = 0; jCol < NCOL; ++jCol) {
            scene(row, jCol).setNoisePos(noisePosFor(row, jCol));
        }
    }

    /** moves the underlying noise (heightmap) by the given offset, recomputes every scene */
    void moveNoise(const glm::vec2& update) {
        noisePosition += update;
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
               scene(iRow, jCol).moveNoise(update);
            }
        }
    }

    void toggleWireFrame() {
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
               scene(iRow, jCol).toggleWireFrame();
            }
        }
    }

    void toggleDebugMode() {
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
               scene(iRow, jCol).toggleDebugMode();
            }
        }
    }

    void cleanup() {
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
               scene(iRow, jCol).cleanup();
            }
        }
    }


private:

    /** the translation to dispay the scene (i,j) in the correct place on screen */
    const glm::vec2& translation(int iRow, int jCol) {
        static const Matrix<glm::vec2> translations = initTranslations();
        //return translations[iRow][jCol];
        return translations[(NROW - rowStart + iRow) % NROW][(NCOL - colStart + jCol) % NCOL];
    }

    /** the scene (i,j) */
    Scene& scene(int iRow, int jCol) {
        static Matrix<Scene> scenes;
        return scenes[iRow][jCol];
    }

    /** the noise position of the scene (i,j) */
    glm::vec2 noisePosFor(int iRow, int jCol) {
        return noisePosition + translation(iRow, jCol);
    }

    /** creates a Matrix of translation vector forming a rectangular matrix */
    Matrix<glm::vec2> initTranslations() {
        Matrix<glm::vec2> t;
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                t[iRow][jCol] = translationScale * glm::vec2(jCol - NCOL / 2, iRow - NROW / 2);
            }
        }
        return t;
    }
};

#endif // SCENE_H
