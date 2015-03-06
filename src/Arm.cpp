#include "Arm.h"
#include <math.h>
#include <iostream>

#define DEG_TO_RAD M_PI/180.f

Arm* Arm::instance = 0;

Arm* Arm::the_instance()
{
    if (!instance) instance = new Arm;
    return instance;
}

Arm::Arm() : num_joints(3),
             component_lengths(num_joints,
                               ARM_LENGTH/num_joints),
             current_angles(num_joints, 0.f),
             max_angles(num_joints, 180.f),
             min_angles(num_joints, -180.f)
{
    min_angles.at(0) = 0.f;
    set_primitive_change(10.f);
}

float Arm::get_joint(int joint_number)
{
    return current_angles.at(joint_number);
}

float Arm::get_component(int joint_number)
{
    return component_lengths.at(joint_number);
}

float Arm::get_joint_max(int joint_number)
{
    return max_angles.at(joint_number);
}

float Arm::get_joint_min(int joint_number)
{
    return min_angles.at(joint_number);
}

pose Arm::get_joints()
{
    return current_angles;
}

length_config Arm::get_components()
{
    return component_lengths;
}

pose Arm::get_max_angles()
{
    return max_angles;
}

pose Arm::get_min_angles()
{
    return min_angles;
}

bool Arm::set_joints(pose angles)
{
    if (angles.size() != num_joints)
    {
        std::cout << "ARM ERROR: Wrong number of joints."
                  << std::endl;
        return false;
    }
    current_angles = angles;
    return true;
}

bool Arm::set_joint(int joint_number, float angle)
{
    if (joint_number < 0 || joint_number >= num_joints)
    {
        std::cout << "ARM ERROR: Invalid joint number."
                  << std::endl;
        return false;
    }
    current_angles.at(joint_number) = angle;
    return true;
}

float Arm::get_ee_x_at(pose position)
{
    return get_joint_x_at(num_joints, position);
}

float Arm::get_ee_y_at(pose position)
{
    return get_joint_y_at(num_joints, position);
}

float Arm::get_joint_x_at(int joint, pose position)
{
    float x = 0.f;
    float angle_sum = 0.f;
    for(int i = 0; i < joint; i++)
    {
        angle_sum += position.at(i);
        x += component_lengths.at(i)*cos((angle_sum)*DEG_TO_RAD);
    }
    return x;
}

float Arm::get_joint_y_at(int joint, pose position)
{
    float y = 0.f;
    float angle_sum = 0.f;
    for(int i = 0; i < joint; i++)
    {
        angle_sum += position.at(i);
        y += component_lengths.at(i)*sin((angle_sum)*DEG_TO_RAD);
    }
    return y;
}

float Arm::get_ee_x()
{
    return get_ee_x_at(current_angles);
}

float Arm::get_ee_y()
{
    return get_ee_y_at(current_angles);
}

bool Arm::is_valid(pose joint_config)
{
    bool valid = true;

    for(int i = 0; i < num_joints; i++)
    {
        // Joints not over max or under min
        if (joint_config.at(i) > max_angles.at(i) ||
            joint_config.at(i) < min_angles.at(i))
        {
            return false;
        }

        // Table collision
        if (get_joint_y_at(i, joint_config) < 0)
        {
            return false;
        }

        //Self collision
        for (int j = 0; j < num_joints; j++)
        {
            if (j == i - 1 || j == i || j == i + 1) continue;
            if (intersect(i, j, joint_config))
            {
                return false;
            }
        }
    }

    return true;
}

bool Arm::is_currently_valid()
{
    return is_valid(current_angles);
}

// http://geomalgorithms.com/a05-_intersect-1.html
bool Arm::intersect(int seg1, int seg2, pose position)
{
    float u_x = (get_joint_x_at(seg1+1, position) -
               get_joint_x_at(seg1, position));

    float u_y = (get_joint_y_at(seg1+1, position) -
               get_joint_y_at(seg1, position));

    float v_x = (get_joint_x_at(seg2+1, position) -
               get_joint_x_at(seg2, position));

    float v_y = (get_joint_y_at(seg2+1, position) -
               get_joint_y_at(seg2, position));

    float w_x = (get_joint_x_at(seg1, position) -
               get_joint_x_at(seg2, position));

    float w_y = (get_joint_y_at(seg1, position) -
               get_joint_y_at(seg2, position));

    float D = u_x*v_y - u_y*v_x;

    if (D < 0.0000001) return false;

    float s = (v_x*w_y - v_y*w_x)/D;
    float t = (u_x*w_y - u_y*w_x)/D;

    if (s < 0 || s > 1) return false;
    if (t < 0 || t > 1) return false;

    return true;
}

bool Arm::apply(action a)
{
    current_angles.at(a.joint) = (current_angles.at(a.joint) +
                                  a.change);
    return is_currently_valid();
}

bool Arm::apply(plan p)
{
    for (plan::iterator i = p.begin(); i != p.end(); i++)
    {
        apply(*i);
    }
    return is_currently_valid();
}

pose Arm::apply_at(action a, pose start)
{
    pose end = start;
    end.at(a.joint) = (start.at(a.joint) +
                       a.change);
    return end;
}

std::vector<action> Arm::get_primitives()
{
    return primitives;
}

void Arm::set_primitive_change(float c)
{
    primitives.clear();
    for (int i = 0; i < num_joints; i++)
    {
        primitives.push_back(action(i, c));
        primitives.push_back(action(i, -c));
    }
}
