#ifndef SCENE_CONTROLER_H
#define SCENE_CONTROLER_H

#include <string>
#include "large_scene.h"

/**
 * @brief The SceneControler class controls the scene redrawing for a smooth displacement effect.
 * The sceneControler moves a cursor on the LargeScene. When the cursor steps outside of the center-scene,
 * the sceneControler draws a new band (column or row) and places the cursor on the center-scene of the redrawn
 * LargeScene. This creates an infinite terrain illusion.
 */
class SceneControler {
public:
    SceneControler(LargeScene& scene, int sceneHalfWidth = 2, int sceneHalfHeight = 2)
        : scene{scene}
        , bound_{sceneHalfWidth, sceneHalfHeight}
    {
    }

    void move(glm::vec2 displacement) {
        position_ += displacement;
        for (int direction : {xAxis,yAxis})
        {
            while(position_[direction] < - bound_[direction]) {
                position_[direction] += bound_[direction];
                moveSceneBand(direction, up(direction));
            }
            while(position_[direction] > bound_[direction]) {
                position_[direction] -= bound_[direction];
                moveSceneBand(direction, down(direction));
            }
        }
    }

    glm::vec2 position() {
        return position_;
    }

private:

    /**
     * Since OpenGL draw the y-axis upside-down, we reverse the direction when it is along the y-axis
     */
    LargeScene::Direction up(int axisDirection) {
        return (axisDirection == xAxis) ? LargeScene::UP : LargeScene::DOWN;
    }

    /**
     * Since OpenGL draw the y-axis upside-down, we reverse the direction when it is along the y-axis
     */
    LargeScene::Direction down(int axisDirection) {
        return (axisDirection == xAxis) ? LargeScene::DOWN : LargeScene::UP;
    }

    /**
     * Choose the right band (column or row) given the `axisDirection`,
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
    glm::vec2 bound_;
    static constexpr int xAxis = 0, yAxis = 1;
};

#endif // SCENE_CONTROLER_H
