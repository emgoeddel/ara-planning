#pragma once

#include <lcm/lcm-cpp.hpp>
#include <iostream>
#include <pthread.h>

#include "ProbCogSearchStates.h"
#include "Search.h"
#include "ProbCogArmCollision.h"
#include "Shortcut.h"

#include "planner_command_t.hpp"
#include "planner_response_t.hpp"
#include "dynamixel_status_list_t.hpp"
#include "dynamixel_command_list_t.hpp"
#include "observations_t.hpp"

typedef std::vector<search_result<arm_state, action> > arastar_result;

enum planner_status{SEARCHING, EXECUTING, WAITING, WAITING_INITIAL};

class planner_interface
{
public:
    planner_interface();

    void handle_command_message(const lcm::ReceiveBuffer* rbuf,
                                const std::string& channel,
                                const planner_command_t* comm);

    void handle_status_message(const lcm::ReceiveBuffer* rbuf,
                               const std::string& channel,
                               const dynamixel_status_list_t* stats);
    void handle_observations_message(const lcm::ReceiveBuffer* rbuf,
                                     const std::string& channel,
                                     const observations_t* obs);

private:
    static void* search_thread(void* arg);
    void search_complete();

    static pthread_t thrd;
    std::vector<object_data_t> latest_objects;
    pose latest_start_pose;
    // For the record, this is so the same plan doesn't get
    // executed twice due to LCM spamming
    bool latest_plan_executed;
    pose arm_status;
    arastar_result latest_search;
    search_request<arm_state, action> latest_request;
    std::vector<action> current_plan;
    planner_status task;
    int current_command_index;
    pose current_command;
    planner_response_t last_response;

    static float PRIMITIVE_SIZE_MIN, PRIMITIVE_SIZE_MAX;
};