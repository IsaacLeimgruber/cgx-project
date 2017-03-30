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

      // TODO 3: Calculate the rotation given the projections of the anchor
      // point and the current position. The rotation axis is given by the cross
      // product of the two projected points, and the angle between them can be
      // used as the magnitude of the rotation.
      // you might want to scale the rotation magnitude by a scalar factor.
      // p.s. No need for using complicated quaternions as suggested inthe wiki
      // article.

      vec3 v1 = normalize(anchor_pos_);
      vec3 v2 = normalize(current_pos);

      vec3 n = cross(v1, v2);
      float angle = acos(dot(v1,v2)) * rotationScale_;
      //std::cout << "Computed angle: " << angle <<std::endl;

      return rotate(IDENTITY_MATRIX, angle, n);
    }

private:
    // projects the point p (whose z coordiante is still empty/zero) onto the
    // trackball surface. If the position at the mouse cursor is outside the
    // trackball, use a hyberbolic sheet as explained in:
    // https://www.opengl.org/wiki/Object_Mouse_Trackball.
    // The trackball radius is given by 'radius_'.
    void ProjectOntoSurface(vec3& p) const {
      // TODO 2: Implement this function. Read above link for details.
        float sqX = p.x * p.x;
        float sqY = p.y * p.y;

        if(sqX + sqY <= halfSqRadius_){
            p.z = sqrt(sqRadius_ - (sqX + sqY));
        } else {
            p.z = (sqRadius_/2.f) / sqrt(sqX + sqY);
        }
        //std::cout << "Projected z: " << p.z <<std::endl;
    }

    float radius_;
    float rotationScale_ = 1.0f;
    float sqRadius_;
    float halfSqRadius_;
    vec3 anchor_pos_;
    mat4 rotation_;
};
