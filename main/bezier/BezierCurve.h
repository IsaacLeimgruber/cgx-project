#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

using namespace glm;
using namespace std;

// This class is essentially a tool to obtain
// a function of t defining a Bezier Curve of 3 points
class BezierCurve{
public:


    BezierCurve(const vector<vec3>& points){
        //computes the sampled points on the Bezier Curve
        for(int i = 0; i < SAMPLE_NUMBER; i++){
            s_points[i] = drawCurve(points, i * TIME_STEP);
        }

        //computes the times at which we pass on each point
        vector<float> smpl = sampled_times(s_points);
        for(int i = 0; i < SAMPLE_NUMBER; i++){
            s_times[i] = smpl[i];
        }
    }

    vec3 getPoint(float global_time){

        //finds between which two point we are meant to be at 'global_time'
        int idx = find_slot(global_time);
        //cout <<"idx "<< idx << endl;
        //calculates the time allocated between p0 and p1
        float time1 = s_times[idx];
        float deltaT = s_times[idx + 1] - time1;
        float slot_time = (global_time - time1)/deltaT; //slot_time in [0, 1] and will be our interpolation factor
        vec3 p0 = s_points[idx];
        vec3 p1 = s_points[idx + 1];
        vec3 res = (1 - slot_time) * p0 + slot_time * p1;
        return res;
    }

    array<float, 300> getTimes(){
        return s_times;
    }

private:
    constexpr static int SAMPLE_NUMBER = 300;
    const float TIME_STEP = 1.f / SAMPLE_NUMBER;
    const float SPEED = 0.005f;

    //Bezier Curve Attributes
    array<vec3, SAMPLE_NUMBER> s_points;
    array<float, SAMPLE_NUMBER> s_times;

    vec3 drawCurve(const vector<vec3>& points, float global_time){
        if(points.size()==1) return points[0];
        else{
            vector<vec3> newpoints(points.size()-1);
            for(size_t i=0; i < newpoints.size(); i++){
                newpoints[i] = (1-global_time) * points[i] + global_time * points[i + 1];
            }
            return drawCurve(newpoints, global_time);
        }
    }

    //returns the indices x, such that times[x] < global and times[x + 1] > global
    int find_slot(float global){
        for(int i = 0; i < SAMPLE_NUMBER - 1; i++){
            if(global >= s_times[i] && global <= s_times[i + 1]){
                return i;
            }
        }
        return -1;
    }


    //Computes the cumulated time at each point
    vector<float> sampled_times(const array<vec3, SAMPLE_NUMBER>& samples){
        vector<float> times;
        float acc = 0.f;
        times.push_back(0.f);
        for(int i = 0; i < SAMPLE_NUMBER-1; i++){

            vec3 p1 = samples[i + 1];
            vec3 p0 = samples[i];
            float distance = glm::length(p1 - p0);
            float time = distance / SPEED;
            acc += time;
            times.push_back(acc);
        }
        for(auto& time : times) {
            time /= acc;
        }
        return times;
    }
};

