#include <lcm/lcm-cpp.hpp>
#include <iostream>
#include <unistd.h>
#include <math.h>

#include "ProbCogSearchStates.h"
#include "Search.h"
#include "ProbCogArmCollision.h"
#include "Shortcut.h"
//#include "RRTStarPlanner.h"
#include "dynamixel_status_list_t.hpp"
#include "dynamixel_command_list_t.hpp"
#include "arm_collision_boxes_t.hpp"
#include "search_target_t.hpp"
#include "observations_t.hpp"

#define PUBLISH_COLLISION_MODEL
//#define SLOW_SPEED

class lcm_handler
{
public:
    lcm_handler() :
        current_command(probcog_arm::get_num_joints(), 0),
        current_command_index(0),
        searching(false) {};
    ~lcm_handler() {};

    void handle_status_message(const lcm::ReceiveBuffer* rbuf,
                               const std::string& channel,
                               const dynamixel_status_list_t* stats)
    {
        if (searching) return;
        lcm::LCM lcm;

        pose np;
        for (int i = 0; i < probcog_arm::get_num_joints(); i++)
        {
            np.push_back(stats->statuses[i].position_radians);
        }
        if (np != status) status = np;

        if (collision_world::collision(status))
        {
            std::cout << "COLLISION" << std::endl;
        }

#ifdef PUBLISH_COLLISION_MODEL
        arm_collision_boxes_t arm_msg = collision_world::arm_boxes(status);
        lcm.publish("ARM_COLLISION_BOXES", &arm_msg);
#endif

        bool done = true;
        for (int i = 0; i < probcog_arm::get_num_joints(); i++)
        {
            if (fabs(status[i] - current_command[i]) > 0.01)
            {
                done = false;
                break;
            }
        }

        if (done && current_plan.size() > 0 &&
            current_command_index < current_plan.size()-1)
        {
            current_command_index++;
            for (int i = 0; i < probcog_arm::get_num_joints(); i++)
            {
                current_command.at(i) +=
                    current_plan.at(current_command_index).at(i);
            }
        }

        dynamixel_command_list_t command;
        command.len = probcog_arm::get_num_joints() + 1;
        for (int i = 0; i < probcog_arm::get_num_joints(); i++)
        {
            dynamixel_command_t c;
            c.position_radians = current_command.at(i);
#ifdef SLOW_SPEED
            c.speed = probcog_arm::get_default_speed(i)*0.1;
#else
            c.speed = probcog_arm::get_default_speed(i);
#endif
            c.max_torque = probcog_arm::get_default_torque(i);
            command.commands.push_back(c);
        }

        dynamixel_command_t hand;
        // if (done && current_plan.size() > 0 &&
        //     current_command_index > current_plan.size())
        // {
            hand.position_radians = 112.f*DEG_TO_RAD;
            hand.speed = 0.15;
            hand.max_torque = 0.5;
        // }
        // else
        // {
        //     hand.position_radians = 0;
        //     hand.speed = 0.15;
        //     hand.max_torque = 0.5;
        // }
        command.commands.push_back(hand);

        lcm.publish("ARM_COMMAND", &command);
    }

    void handle_observations_message(const lcm::ReceiveBuffer* rbuf,
                                     const std::string& channel,
                                     const observations_t* obs)
    {
        latest_objects = obs->observations;
        collision_world::clear();
        for (std::vector<object_data_t>::iterator i =
                 latest_objects.begin();
             i != latest_objects.end(); i++)
        {
            collision_world::add_object(i->bbox_dim, i->bbox_xyzrpy);
        }
    }

    void handle_target_message(const lcm::ReceiveBuffer* rbuf,
                               const std::string& channel,
                               const search_target_t* targ)
    {
        searching = true;

        lcm::LCM lcm;
        point_3d goal;
        for (int i = 0; i < 3; i++)
        {
            goal.push_back(targ->target[i]);
        }

        std::cout << "About to search" << std::endl;
        arm_state::target = goal;
        arm_state::pitch_matters = false;
        std::vector<search_result<arm_state, action> > latest_search;

        search_request<arm_state, action> req(arm_state(status),
                                              probcog_arm::big_primitives(),
                                              probcog_arm::small_primitives());


        arastar<arm_state, action>(req);
        latest_search = req.copy_solutions();
        current_plan = latest_search.at(latest_search.size()-1).path;
        current_plan = shortcut<arm_state, action>(current_plan,
                                                   arm_state(status));
        std::cout << "Shortcutted to " << current_plan.size()
                  << std::endl;

        current_command = status;
        for (int i = 0; i < probcog_arm::get_num_joints(); i++)
        {
            current_command.at(i) += current_plan.at(0).at(i);
        }
        current_command_index = 0;
        searching = false;
    }

    pose status;
    pose current_command;
    int current_command_index;
    bool searching;
    std::vector<object_data_t> latest_objects;
    std::vector<action> current_plan;
};

int main(int argc, char* argv[])
{
    lcm::LCM lcm;
    if (!lcm.good())
    {
        std::cout << "Failed to initialize LCM." << std::endl;
        return 1;
    }

    probcog_arm::INIT();

    lcm_handler handler;
    lcm.subscribe("ARM_STATUS", &lcm_handler::handle_status_message,
                  &handler);
    lcm.subscribe("SEARCH_TARGET", &lcm_handler::handle_target_message,
                  &handler);
    lcm.subscribe("OBSERVATIONS", &lcm_handler::handle_observations_message,
                  &handler);

    while(0 == lcm.handle());

    return 0;
}
