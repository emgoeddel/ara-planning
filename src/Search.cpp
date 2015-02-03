#include "Search.h"
#include <math.h>
#include <queue>

Search::Search() : arm(2),
                   target(arm.get_ee_x(), arm.get_ee_y(), 0, 0)
{
    for (int i = 0; i < arm.get_num_joints(); i++)
    {
        primitives.insert(action(i, 10.f));
    }
}

Search::Search(Arm& start) :
    arm(start),
    target(arm.get_ee_x(), arm.get_ee_y(), 0, 0)
{
    for (int i = 0; i < arm.get_num_joints(); i++)
    {
        primitives.insert(action(i, 10.f));
    }
}

Search::Search(int arm_num_joints) :
    arm(arm_num_joints),
    target(arm.get_ee_x(), arm.get_ee_y(), 0, 0)
{
    for (int i = 0; i < arm.get_num_joints(); i++)
    {
        primitives.insert(action(i, 10.f));
    }
}

float Search::euclidean_heuristic()
{
    return euclidean_heuristic(arm.get_joints());
}

float Search::euclidean_heuristic(pose position)
{
    float x = arm.get_ee_x_at(position);
    float y = arm.get_ee_y_at(position);

    return sqrt(pow(target.x-x, 2) + pow(target.y-y, 2));
}

const Arm Search::get_current_arm()
{
    return arm;
}

void Search::set_arm(Arm& a)
{
    arm = a;
}

void Search::set_arm_position(pose angles)
{
    arm.set_joints(angles);
}

void Search::set_arm_num_joints(int num_joints)
{
    arm = Arm(num_joints);
}

void Search::set_target(target_t target)
{
    target = target;
}

void Search::set_target(float x, float y, float err_x, float err_y)
{
    target = target_t(x, y, err_x, err_y);
}

bool Search::is_in_goal(pose pos)
{
    if(arm.get_ee_x() > (target.x + target.err_x) ||
       arm.get_ee_x() < (target.x - target.err_x))
    {
        return false;
    }
    if(arm.get_ee_y() > (target.y + target.err_y) ||
       arm.get_ee_y() < (target.y - target.err_y))
    {
        return false;
    }

    return true;
}

plan Search::astar(Arm start, target_t target)
{
    set_arm(start);
    set_target(target);
    return astar();
}

plan Search::astar(target_t target)
{
    set_target(target);
    return astar();
}

plan Search::astar(pose start, target_t target)
{
    set_arm_position(start);
    set_target(target);
    return astar();
}

plan Search::astar()
{
    plan p;

    std::priority_queue<node> open;
    std::set<pose> expanded;

    if(is_in_goal(arm.getJoints()))
    {
        p.push_back(action(0,0));
        return p;
    }

    node start(arm.getJoints(), action(0,0), NULL);
    node end;

    while (!expanded_goal)
    {
        node current = open.top();
        open.pop();

        for (std::set<action>::iterator it=primitives.begin();
             it != primitives.end(); it++)
        {
            pose successor(current.joints);
            successor.at(it->joint) = (successor.at(it->joint) +
                                       it->change);

            if (!arm.is_valid(successor)) continue;

            if (!expanded.count(successor) ||)


            if (is_in_goal(successor))
            {
                expanded_goal = true;
            }
        }

        open
    }

    /////////////////////
    p.push_back(pose(arm.get_num_joints(), 0.f));
    return p;
}
