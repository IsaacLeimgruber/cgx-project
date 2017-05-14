#ifndef LSCENE_H
#define LSCENE_H

#include <array>
#include <iostream>
#include "water/water.h"
#include "terrain/terrain.h"
#include "perlin/perlin.h"

/** A LargeScene is an infinite procedural terrain. Internally, it is a circular grid of Scene objects */
class LargeScene {

    /** the dimensions of the large scene's rectangular matrix */
    enum {NROW = 6, NCOL = 6};

    /** the dimension of a small scene as seen per the scene's vertex shader 2 = size([-1;1]) */
    const float gridSize = 2.0f;

    template <class T> using Row = std::array<T, NCOL>;
    template <class T> using Matrix = std::array<Row<T>, NROW>;

    /** the matrix i-coordinate of the scene displayed in the bottom left corner */
    int rowStart = 0;

    /** the matrix j-coordinate of the scene displayed in the bottom left corner */
    int colStart = 0;

    /** the noise position of the scene displayed in the bottom left corner */
    glm::vec2 noisePosition {0, 0};

    Grid grid;
    Water water;
    Perlin perlin;
    int heightMapWidth, heightMapHeight;

public:
    enum Direction { UP = +1, DOWN = -1 };

    /** initializes the perlin textures */
    void initPerlin(int textureWidth = 1024, int textureHeight = 1024) {
        heightMapWidth = textureWidth;
        heightMapHeight = textureHeight;
        perlin.Init();
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                heightMap(iRow, jCol).Init(textureWidth, textureHeight, GL_RGB32F, GL_RGB, GL_FLOAT, true);
                recomputeHeightMap(iRow, jCol);
            }
        }
    }

    /** initializes the scenes object */
    void init(int shadowBuffer_texture_id, int reflectionBuffer_texture_id, Light* light) {
        grid.Init(0, shadowBuffer_texture_id);
        water.Init(0, reflectionBuffer_texture_id, shadowBuffer_texture_id);
        grid.useLight(light);
        water.useLight(light);
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
                grid.useHeightMap(heightMap(iRow, jCol).id());
                grid.Draw(MVP, MV, NORMALM, SHADOWMVP, FV,
                                      mirrorPass, shadowPass,
                                      gridSize * translation(iRow, jCol));
            }
        }

        if(!mirrorPass && !shadowPass)
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
               water.useHeightMap(heightMap(iRow, jCol).id());
               water.Draw(MVP, MV, NORMALM, SHADOWMVP, FV,
                                      noisePosFor(iRow, jCol),
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
            recomputeHeightMap(iRow, col);
        }
    }

    /** moves the scenes one row in the given direction, recomputes only obsolete scenes */
    void moveRows(Direction d) {
        int oldRowStart = rowStart;
        rowStart = (rowStart - d + NROW) % NROW;
        noisePosition.y -= d;

        int row = (d == DOWN) ? oldRowStart : rowStart;
        for(int jCol = 0; jCol < NCOL; ++jCol) {
            recomputeHeightMap(row, jCol);
        }
    }

    void toggleWireFrame() {
        water.toggleWireFrame();
        grid.toggleWireFrame();
    }

    void toggleDebugMode() {
        water.toggleDebugMode();
        grid.toggleDebugMode();
    }

    float maximumExtent(){
        return 1.42 * max(NROW, NCOL) * 2;
    }

    void cleanup() {
        water.Cleanup();
        grid.Cleanup();
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
               heightMap(iRow, jCol).Cleanup();
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

    /** the heightMap (i,j) */
    ColorFBO& heightMap(int iRow, int jCol) {
        static Matrix<ColorFBO> maps;
        return maps[iRow][jCol];
    }

    /** redraw the perlin noise inside appropriate buffer */
    void recomputeHeightMap(int iRow, int jCol) {
        heightMap(iRow, jCol).Bind();
        perlin.Draw(noisePosFor(iRow, jCol));
        heightMap(iRow, jCol).Unbind();
    }

    /** the noise position of the scene (i,j) */
    glm::vec2 noisePosFor(int iRow, int jCol) {
        return textureCorrection(noisePosition + translation(iRow, jCol));
    }

    /** reduces the offset of one pixel for perfect heightMap juxtaposition */
    glm::vec2 textureCorrection(glm::vec2 pos_offset) {
        return {
            pos_offset.x * (1 - 1.0 / heightMapWidth),
            pos_offset.y * (1 - 1.0 / heightMapHeight)
        };
    }

    /** creates a Matrix of translation vector forming a rectangular matrix */
    Matrix<glm::vec2> initTranslations() {
        Matrix<glm::vec2> t;
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                t[iRow][jCol] = glm::vec2(jCol - NCOL / 2, iRow - NROW / 2);
            }
        }
        return t;
    }
};

#endif // SCENE_H
