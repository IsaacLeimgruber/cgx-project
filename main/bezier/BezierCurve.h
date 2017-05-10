/*
 * Code taken and adapted from https://learnopengl.com/#!Getting-started/Camera
 */

#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

const int SAMPLE_NUMBER = 20;
const float TIME_STEP = 1.f / SAMPLE_NUMBER;
const float SPEED = 1.f;

// This class is essentially a tool to obtain
// a function of t defining a Bezier Curve of 3 points
class BezierCurve{
public:
    //Bezier Curve Attributes
    vec3 startingPoint;
    vec3 endingPoint;
    vec3 controlPoints[20];
    int controlPoints_s;


BezierCurve(vec3 spoint, vec3 epoint, vec3 cpoints[], int cpoints_size){
    this->startingPoint = spoint;
    this->endingPoint = epoint;
    for(int i = 0; i < cpoints_size; i++){
        controlPoints[i] = cpoints[i];
    }
    this->controlPoints_s = cpoints_size;
}

private:

    //t must be in [0, 1] to respect the linear interpolation
    vec3 poly_deg_3(float t, vec3 start, vec3 mid1, vec3 mid2, vec3 end){
        float t2 = t * t;
        float t3 = t2 * t;
        float mt = (1 - t);
        float mt2 = mt * mt;
        float mt3 = mt2 * mt;
        return start * mt3 +
               3 * mid1 * mt2 * t +
               3 * mid2 * mt * t2 +
               end * t3;
    }

    vec3 poly_deg_2(float t, vec3 start, vec3 mid, vec3 end){
          float t2 = t * t;
          float mt = 1-t;
          float mt2 = mt * mt;
          return start * mt2 +
                 mid * 2 * mt * t +
                 end * t2;
    }

    typedef vec3 (*fptr)();
    fptr bezier3(){
        return poly_deg_3;
    }
    fptr bezier2(){
        return poly_deg_2;
    }

    vec3* sampled_points(){
        vec3 samples[SAMPLE_NUMBER] = {};
        for(int i = 0; i < SAMPLE_NUMBER; i++){
            samples[i] = bezier2()(i * TIME_STEP);
        }
        return samples;
    }

    //you can access times or dir respectively by myPair.first, myPair.second after function call
    pair<float , vec3>* sampled_times_dir(float* samples){
        float times_dirs[SAMPLE_NUMBER - 1] = {};
        for(int i = 0; i < SAMPLE_NUMBER - 1; i++){
            vec3 p1 = samples[i + 1];
            vec3 p0 = samples[i];
            vec3 dir = p1 - p0;
            float distance = norm(dir);
            times_dirs[i].second = dir / distance;
            times_dirs[i].first = distance / SPEED;
        }
        return times_dirs;
    }


};

