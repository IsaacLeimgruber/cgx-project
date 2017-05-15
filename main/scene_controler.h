#ifndef SCENE_CONTROLER_H
#define SCENE_CONTROLER_H

#include <string>
#include "large_scene.h"

/**
 * @brief The SceneControler class controls the Grid redrawing for a smooth displacement effect.
 * The sceneControler moves a cursor on the LargeScene. When the cursor steps outside of the center-grid,
 * the sceneControler draws a new band (column or row) and places the cursor on the center-grid of the redrawn
 * LargeScene. This creates an infinite terrain illusion.
 */
class SceneControler {
public:
    SceneControler(LargeScene& scene, int gridHalfWidth = 1, int gridHalfHeight = 1)
        : scene{scene}
        , gridHalfDim_{gridHalfWidth, gridHalfHeight}
    {
    }

    /**
     * This function replaces the cursor (X) inside the middle Grid
     *
     * Before:
     * > The middle Grid is `1`
     * > The cursor (X) is outside of the middle Grid (X.x > gridHalfDim)
     *
     *        _
     * |------1-------|-------2------|-------3------|
     * |              |              |              |
     * |              |X             |              |
     * |              |              |              |
     * |              |              |              |
     * |--------------|--------------|--------------|
     *         <------>
     *         0      gridHalfDim
     *
     *
     * After:
     * > The middle Grid is `2`
     * > The cursor is inside the middle Grid, near the opposite border (X.x -= 2 * gridHalfDim)
     *
     *        _
     * |------2-------|-------3------|
     * |              |              |
     * |X             |              |
     * |              |              |
     * |              |              |
     * |--------------|--------------|
     *         <------>
     *         0      gridHalfDim
     */
    void move(glm::vec2 displacement) {
        position_ += displacement;
        for (int direction : {xAxis,yAxis})
        {
            while(position_[direction] < - gridHalfDim_[direction]) {
                position_[direction] += 2*gridHalfDim_[direction];
                moveSceneBand(direction, up(direction));
            }
            while(position_[direction] > gridHalfDim_[direction]) {
                position_[direction] -= 2*gridHalfDim_[direction];
                moveSceneBand(direction, down(direction));
            }
        }
    }

    glm::vec2 position() {
        return position_;
    }

private:

    /**
     * Since OpenGL draws the y-axis upside-down, we reverse the direction when it is along the y-axis
     */
    LargeScene::Direction up(int axisDirection) {
        return (axisDirection == xAxis) ? LargeScene::UP : LargeScene::DOWN;
    }

    /**
     * Since OpenGL draws the y-axis upside-down, we reverse the direction when it is along the y-axis
     */
    LargeScene::Direction down(int axisDirection) {
        return (axisDirection == xAxis) ? LargeScene::DOWN : LargeScene::UP;
    }

    /**
     * Chooses the right band (column or row) given the `axisDirection`,
     * And moves it UP or DOWN accordingly to `updown`
     */
    void moveSceneBand(int axisDirection, LargeScene::Direction updown) {
        switch (axisDirection) {
        case xAxis:
            scene.moveCols(updown);
            return;
        case yAxis:
            scene.moveRows(updown);
            return;
        default:
            throw std::runtime_error{"Unknown direction " + std::to_string(axisDirection)};
        }
    }

    LargeScene& scene;
    glm::vec2 position_;
    glm::vec2 gridHalfDim_;
    static constexpr int xAxis = 0, yAxis = 1;
};

#endif // SCENE_CONTROLER_H
