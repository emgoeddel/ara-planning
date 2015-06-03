#pragma once

#include <vector>

#include "fcl/collision.h"
#include "fcl/broadphase/broadphase.h"

#include "ProbCogArm.h"

bool collision_function(fcl::CollisionObject* o1,
                        fcl::CollisionObject* o2, void* cdata);

struct collision_data
{
    collision_data()
    {
        done = false;
    }

    fcl::CollisionRequest request;
    fcl::CollisionResult result;
    bool done;
};


class collision_world
{
public:
    static void add_object(std::vector<float> dim,
                           std::vector<float> xyzrpy);

    bool collision(pose arm_position);

private:
    collision_world() {};
    collision_world(collision_world const&) {};
    collision_world& operator=(collision_world const&) {};

    static fcl::BroadPhaseCollisionManager* world_objects_m;
    static fcl::BroadPhaseCollisionManager* arm_objects_m;
};