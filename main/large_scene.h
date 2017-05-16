#ifndef LSCENE_H
#define LSCENE_H

#include <array>
#include <iostream>
#include "water/water.h"
#include "terrain/terrain.h"
#include "perlin/perlin.h"

/** A LargeScene is an infinite procedural terrain. Internally, it is a circular matrix of Grid objects */
class LargeScene {

    /** the dimensions of this LargeScene's rectangular matrix */
    enum {NROW = 10, NCOL = 10};

    /** the dimension of the Grid as seen per its vertex shader 2 = size([-1;1]) */
    const float gridSize = 2.0f;

    template <class T> using Row = std::array<T, NCOL>;
    template <class T> using Matrix = std::array<Row<T>, NROW>;

    /** the matrix i-coordinate of the Grid displayed in the bottom left corner */
    int rowStart = 0;

    /** the matrix j-coordinate of the Grid displayed in the bottom left corner */
    int colStart = 0;

    /** the noise position of the Grid displayed in the bottom left corner */
    glm::vec2 noisePosition {0, 0};

    /** the grid tile we reuse at each (i,j) position */
    Grid grid;

    /** the grid's water tile we reuse at each (i,j) position */
    Water water;

    /** the noise algorithm */
    Perlin perlin;

    /** resolution of the height maps */
    int heightMapWidth, heightMapHeight;

public:
    enum Direction { UP = +1, DOWN = -1 };

    struct TileSet {
        vector<const pair<int, int>> tiles;
    };

    /** initializes the heightMaps */
    void initHeightMap(int textureWidth = 1024, int textureHeight = 1024) {
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

    /** initializes the tile objects (grid, water, etc.) */
    void init(int shadowBuffer_texture_id, int reflectionBuffer_texture_id, Light* light) {
        grid.Init(0, shadowBuffer_texture_id);
        water.Init(0, reflectionBuffer_texture_id, shadowBuffer_texture_id);
        grid.useLight(light);
        water.useLight(light);
    }

    /** draws every grid tile side by side in an ordered manner */
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

    void writeVisibleTilesOnly(TileSet& visible, const glm::vec3 &pointInPlane, const glm::vec3 &planeNormal)
    {
        visible.tiles.clear();
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                glm::vec2 tileCenter2D = gridSize * translation(iRow, jCol);
                glm::vec3 tileCenter = glm::vec3(tileCenter2D.x, 0, -tileCenter2D.y);
                glm::vec3 tileCorner = tileCenter + glm::vec3(sgn(planeNormal.x), 0, sgn(planeNormal.z));
                glm::vec3 pointToCorner = tileCorner - pointInPlane;
                bool isVisible = glm::dot(pointToCorner, planeNormal) > 0;
                if (isVisible) {
                    visible.tiles.push_back({iRow, jCol});
                }
            }
        }
    }

    /** draws every non-culled mountain tile side by side in an ordered manner */
    void drawMountainTiles(
            TileSet const& tilesToDraw,
            const glm::mat4 &MVP = IDENTITY_MATRIX,
            const glm::mat4 &MV = IDENTITY_MATRIX,
            const glm::mat4 &NORMALM = IDENTITY_MATRIX,
            const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
            const FractionalView &FV = FractionalView(),
            bool mirrorPass = false)
    {
        for (auto&& i : tilesToDraw.tiles) {
            grid.useHeightMap(heightMap(i.first, i.second).id());
            grid.Draw(MVP, MV, NORMALM, SHADOWMVP, FV,
                      mirrorPass, false,
                      gridSize * translation(i.first, i.second));

        }
    }

    /** draws every non-culled water tile side by side in an ordered manner */
    void drawWaterTiles(
            TileSet const& tilesToDraw,
            const glm::mat4 &MVP = IDENTITY_MATRIX,
            const glm::mat4 &MV = IDENTITY_MATRIX,
            const glm::mat4 &NORMALM = IDENTITY_MATRIX,
            const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
            const FractionalView &FV = FractionalView())
    {
        for (auto&& i : tilesToDraw.tiles)  {
            water.useHeightMap(heightMap(i.first, i.second).id());
            water.Draw(MVP, MV, NORMALM, SHADOWMVP, FV,
                       noisePosFor(i.first, i.second),
                       gridSize * translation(i.first, i.second));

        }
    }

    /** moves the heightMaps one column in the given direction, recomputes only obsolete heightMaps */
    void moveCols(Direction d) {
        int oldColStart = colStart;
        colStart = (colStart - d + NCOL) % NCOL;
        noisePosition.x -= d;

        int col = (d == DOWN) ? oldColStart : colStart;
        for(int iRow = 0; iRow < NROW; ++iRow) {
            recomputeHeightMap(iRow, col);
        }
    }

    /** moves the heightMaps one row in the given direction, recomputes only obsolete heightMaps */
    void moveRows(Direction d) {
        int oldRowStart = rowStart;
        rowStart = (rowStart - d + NROW) % NROW;
        noisePosition.y -= d;

        int row = (d == DOWN) ? oldRowStart : rowStart;
        for(int jCol = 0; jCol < NCOL; ++jCol) {
            recomputeHeightMap(row, jCol);
        }
    }

    /** A circle with diameter maximumExtent can contain this whole LargeScene */
    float maximumExtent(){
        constexpr float sqrt2 = 1.42;
        return sqrt2 * max(NROW, NCOL) * gridSize;
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
        water.Cleanup();
        grid.Cleanup();
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                heightMap(iRow, jCol).Cleanup();
            }
        }
    }

private:
    /** the translation to dispay the grid (i,j) at the correct place on screen */
    const glm::vec2& translation(int iRow, int jCol) {
        static const Matrix<glm::vec2> translations = initTranslations();
        //return translations[iRow][jCol];
        return translations[(NROW - rowStart + iRow) % NROW][(NCOL - colStart + jCol) % NCOL];
    }

    /** the height map buffer (i,j) */
    ColorFBO& heightMap(int iRow, int jCol) {
        static Matrix<ColorFBO> maps;
        return maps[iRow][jCol];
    }

    /** redraws the perlin noise inside appropriate height map buffer */
    void recomputeHeightMap(int iRow, int jCol) {
        heightMap(iRow, jCol).Bind();
        perlin.Draw(textureCorrection(noisePosFor(iRow, jCol)));
        heightMap(iRow, jCol).Unbind();
    }

    /** the noise position of the grid (i,j) */
    glm::vec2 noisePosFor(int iRow, int jCol) {
        return noisePosition + translation(iRow, jCol);
    }

    /** reduces the offset of one pixel for perfect heightMap texture juxtaposition */
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

    static float sgn(float val) {
        return (0.0 < val) - (val < 0.0);
    }
};

#endif // LSCENE_H
