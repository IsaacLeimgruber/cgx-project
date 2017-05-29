#ifndef LSCENE_H
#define LSCENE_H

#include <array>
#include <iostream>
#include <algorithm>
#include "water/water.h"
#include "terrain/terrain.h"
#include "perlin/perlin.h"
#include "grass/grass.h"
#include "model/model.h"

/** A LargeScene is an infinite procedural terrain. Internally, it is a circular matrix of Grid objects */
class LargeScene {

    /** the dimensions of this LargeScene's rectangular matrix in number of tiles per ROW or COLumns */
    enum {NROW = 15, NCOL = 15};

    /** the number of tile to make transparent at the edge of the large scene */
    static constexpr int nMountainTilesInFog = 3;
    static constexpr int nWaterTilesInFog = nMountainTilesInFog;
    static constexpr int fogStop = (NROW < NCOL ? NROW : NCOL) - 1;

    /** the dimension of the Grid as seen per its vertex shader 2 = size([-1;1]) */
    float gridSize = 2.0f;
    float worldGridSize;

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

    /** the grid's grass tile we reuse at each (i,j) position */
    Grass grass;

    /** the noise algorithm */
    Perlin perlin;

    /** A glorious ship with its shader program */
    Model mightyShip{"yacht.3ds"};
    GLuint mightyShipShaderProgram;
    glm::mat4 shipModelMatrix;
    //glm::vec2 shipWorldPos{}

    /** resolution of the height maps */
    int heightMapWidth, heightMapHeight;

    /** resolution of the grass maps */
    int grassMapWidth, grassMapHeight;

    /** this large scene's center */
    glm::vec2 center;

public:
    enum Direction { UP = +1, DOWN = -1 };

    LargeScene(float worldGridSize) : worldGridSize{worldGridSize} {}

    struct Index {
        int iRow;
        int jCol;
    };

    struct TileSet {
        vector<pair<Index, float>> tiles;
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

    /** initializes the grassMaps */
    void initGrassMap(int textureWidth = 1024, int textureHeight = 1024) {
        grassMapWidth = textureWidth;
        grassMapHeight = textureHeight;
        perlin.Init("perlinGrass_fshader.glsl");
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                grassMap(iRow, jCol).Init(textureWidth, textureHeight, GL_RGB32F, GL_RGB, GL_FLOAT, true);
                recomputeGrassMap(iRow, jCol);
            }
        }
    }

    /** initializes the tile objects (grid, water, etc.) */
    void init(int shadowBuffer_texture_id, int reflectionBuffer_texture_id, Light* light) {
        grass.Init(0 /*heightMap*/, 0 /*grassMap*/);
        grid.Init(0, shadowBuffer_texture_id, 0, fogStop, nMountainTilesInFog);
        water.Init(0, reflectionBuffer_texture_id, shadowBuffer_texture_id, fogStop, nWaterTilesInFog);
        grid.useLight(light);
        water.useLight(light);
        grass.useLight(light);

        mightyShipShaderProgram = icg_helper::LoadShaders("yacht_vshader.glsl", "yacht_fshader.glsl");
        mightyShip.Init(mightyShipShaderProgram, shadowBuffer_texture_id, fogStop, nMountainTilesInFog);
        mightyShip.useLight(light);
    }

    /** draws every Mountain grid tile side by side in an ordered manner */
    void drawMountains(const glm::mat4 &MVP = IDENTITY_MATRIX,
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
                grid.useGrassMap(grassMap(iRow, jCol).id());
                grid.Draw(MVP, MV, NORMALM, SHADOWMVP, FV,
                          mirrorPass, shadowPass,
                          gridSize * translation(iRow, jCol));
            }
        }
    }

    void writeVisibleTilesOnly(TileSet& visible, const glm::vec3 &pointInPlane, const glm::vec3 &planeNormal)
    {
        visible.tiles.clear();
        for (int iRow = 0; iRow < NROW; ++iRow) {
            for (int jCol = 0; jCol < NCOL; ++jCol) {
                glm::vec2 tileCenter2D = worldGridSize * translation(iRow, jCol);
                glm::vec3 tileCenter = glm::vec3(tileCenter2D.x, 0, -tileCenter2D.y);
                glm::vec3 tileCorner = tileCenter + (worldGridSize / 2) * glm::vec3(sgn(planeNormal.x), 0, sgn(planeNormal.z));
                glm::vec3 pointToCorner = tileCorner - pointInPlane;
                float depth = glm::dot(tileCenter - pointInPlane, tileCenter - pointInPlane);
                bool isVisible = glm::dot(pointToCorner, planeNormal) > 0;
                if (isVisible) {
                    visible.tiles.push_back({Index{iRow, jCol}, depth});
                }
            }
        }

        //sort tiles to that tiles nearer to pointInPlane are drawn first
        std::sort(visible.tiles.begin(), visible.tiles.end(),
                  [](pair<Index, float> const& a, pair<Index, float> const& b) {
            return a.second < b.second;
        });

        //scale depth between -1 and 1
        if (visible.tiles.back().second - visible.tiles.front().second > 1) {
            float inv_maxDepth = 1.0f / (visible.tiles.back().second - visible.tiles.front().second);
            for (auto& tile : visible.tiles) {
                tile.second -= visible.tiles.front().second;
                tile.second *= inv_maxDepth;
            }
        } else {
            for (auto& tile : visible.tiles) {
                tile.second = 0;
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
            grid.useHeightMap(heightMap(i.first.iRow, i.first.jCol).id());
            grid.useGrassMap(grassMap(i.first.iRow, i.first.jCol).id());
            grid.Draw(MVP, MV, NORMALM, SHADOWMVP, FV,
                      mirrorPass, false,
                      gridSize * translation(i.first.iRow, i.first.jCol),
                      gridSize * translation(i.first.iRow, i.first.jCol) - center);

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
            water.useHeightMap(heightMap(i.first.iRow, i.first.jCol).id());
            water.Draw(MVP, MV, NORMALM, SHADOWMVP, FV,
                       noisePosFor(i.first.iRow, i.first.jCol),
                       gridSize * translation(i.first.iRow, i.first.jCol),
                       gridSize * translation(i.first.iRow, i.first.jCol) - center);
        }
    }

    void drawGrassTiles(TileSet const& tilesToDraw,
                        const mat4 &VP = IDENTITY_MATRIX,
                        const vec2 &cameraPos = vec2(0.f, 0.f)) {
        for (auto&& i : tilesToDraw.tiles)  {
            grass.useHeightMap(heightMap(i.first.iRow, i.first.jCol).id());
            grass.Draw(VP,
                       gridSize * translation(i.first.iRow, i.first.jCol),
                       gridSize * translation(i.first.iRow, i.first.jCol) - center,
                       cameraPos);
        }
    }

    void drawModels(
            const glm::mat4 &MVP = IDENTITY_MATRIX,
            const glm::mat4 &MV = IDENTITY_MATRIX,
            const glm::mat4 &SHADOWMVP = IDENTITY_MATRIX,
            bool mirrorPass = false){

      float time = glfwGetTime();
      float pitching = cos(time * 0.9) * 0.10;

      glm::vec3 shipPos = glm::vec3(
                  6.0 - noisePosition.x * worldGridSize,
                  0.035,
                  12.0 + noisePosition.y * worldGridSize);

      shipModelMatrix =
              glm::translate(IDENTITY_MATRIX, shipPos)
              *
              glm::rotate(IDENTITY_MATRIX, pitching, glm::vec3(1.0, 0.0, 0.0));


      //std::cout << (-noisePosition.x * worldGridSize) << " : " << (noisePosition.y* worldGridSize) << std::endl;
      glm::mat4 shipMVP = MVP * shipModelMatrix;
      glm::mat4 shipMV = MV * shipModelMatrix;
      glm::mat4 shipNORMALM = inverse(transpose(shipMV));
      glDisable(GL_CULL_FACE);
      mightyShip.Draw(shipMVP, shipMV, shipNORMALM, SHADOWMVP, glm::vec2(shipPos.x, -shipPos.z) - center, mirrorPass);
      glEnable(GL_CULL_FACE);
    }

    /** moves the heightMaps one column in the given direction, recomputes only obsolete heightMaps */
    void moveCols(Direction d) {
        int oldColStart = colStart;
        colStart = (colStart - d + NCOL) % NCOL;
        noisePosition.x -= d;

        int col = (d == DOWN) ? oldColStart : colStart;
        for(int iRow = 0; iRow < NROW; ++iRow) {
            recomputeHeightMap(iRow, col);
            recomputeGrassMap(iRow, col);
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
            recomputeGrassMap(row, jCol);
        }
    }

    /** A circle with diameter maximumExtent can contain this whole LargeScene */
    float maximumExtent(){
        constexpr float sqrt2 = 1.42;
        return sqrt2 * std::max(NROW- (nMountainTilesInFog / 2), NCOL- (nMountainTilesInFog / 2)) * gridSize;
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
                grassMap(iRow, jCol).Cleanup();
            }
        }
    }

    void setCenter(glm::vec2 c) {
        center = c;
        center.y = -center.y;
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
        perlin.Draw(textureCorrection(noisePosFor(iRow, jCol), heightMapWidth, heightMapHeight));
        heightMap(iRow, jCol).Unbind();
    }

    /** the grass map buffer (i,j) */
    ColorFBO& grassMap(int iRow, int jCol) {
        static Matrix<ColorFBO> maps;
        return maps[iRow][jCol];
    }

    /** redraws the perlin noise inside appropriate grass map buffer */
    void recomputeGrassMap(int iRow, int jCol) {
        grassMap(iRow, jCol).Bind();
        perlin.Draw(textureCorrection(noisePosFor(iRow, jCol), grassMapWidth, grassMapHeight));
        grassMap(iRow, jCol).Unbind();
    }

    /** the noise position of the grid (i,j) */
    glm::vec2 noisePosFor(int iRow, int jCol) {
        return noisePosition + translation(iRow, jCol);
    }

    /** reduces the offset of one pixel for perfect texture juxtaposition */
    glm::vec2 textureCorrection(glm::vec2 pos_offset, float width, float height) {
        return {
            pos_offset.x * (1 - 1.0 / width),
                    pos_offset.y * (1 - 1.0 / height)
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
