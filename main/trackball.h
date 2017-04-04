#pragma once
#include "icg_helper.h"


using namespace glm;

class Trackball {
public:
    Trackball() : radius_(1.0f) {
        sqRadius_ = radius_* radius_;
        halfSqRadius_ = sqRadius_ * 0.5f;
    }

    // this function is called when the user presses the left mouse button down.
    // x, and y are in [-1, 1]. (-1, -1) is the bottom left corner while (1, 1)
    // is the top right corner.
    void BeingDrag(float x, float y) {
      anchor_pos_ = vec3(x, y, 0.0f);
      ProjectOntoSurface(anchor_pos_);
    }

    // this function is called while the user moves the curser around while the
    // left mouse button is still pressed.
    // x, and y are in [-1, 1]. (-1, -1) is the bottom left corner while (1, 1)
    // is the top right corner.
    // returns the rotation of the trackball in matrix form.
    mat4 Drag(float x, float y) {

      vec3 current_pos = vec3(x, y, 0.0f);
      ProjectOntoSurface(current_pos);

      vec3 v1 = normalize(anchor_pos_);
      vec3 v2 = normalize(current_pos);

      vec3 n = cross(v1, v2);
      float angle = acos(dot(v1,v2)) * rotationScale_;
      std::cout << "Computed angle: " << angle <<std::endl;

      return rotate(IDENTITY_MATRIX, angle, n);
    }

private:

    void ProjectOntoSurface(vec3& p) const {

        float sqX = p.x * p.x;
        float sqY = p.y * p.y;

        if(sqX + sqY <= halfSqRadius_){
            p.z = sqrt(sqRadius_ - (sqX + sqY));
        } else {
            p.z = (sqRadius_/2.f) / sqrt(sqX + sqY);
        }
        std::cout << "Projected z: " << p.z <<std::endl;
    }

    float radius_;
    float rotationScale_ = 1.0f;
    float sqRadius_;
    float halfSqRadius_;
    vec3 anchor_pos_;
    mat4 rotation_;
};
