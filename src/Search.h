#pragma once

#include "Arm.h"

struct target_t
{
    float x;
    float y;
    float err_x;
    float err_y;

    target_t(float x, float y, float ex, float ey): x(x),
                                                    y(y),
                                                    err_x(ex),
                                                    err_y(ey) {}
};

class Search
{
public:
    Search();
    Search(Arm& start);
    Search(int arm_num_joints);

    float get_target_x() {return target.x;}
    float get_target_y() {return target.y;}
    target_t get_target() {return target;}

    const Arm get_current_arm();
    void set_arm(Arm a);
    void set_arm_position(std::vector<float> angles);
    void set_arm_num_joints(int num_joints);

    void set_target(target_t target);
    void set_target(float x, float y, float err_x, float err_y);

    std::vector<float> astar(Arm start, target_t target);
    std::vector<float> astar(target_t target);
    std::vector<float> astar(std::vector<float> start,
                             target_t target);
    std::vector<float> astar();

private:
    Arm arm;
    target_t target;
};
