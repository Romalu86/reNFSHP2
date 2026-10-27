#include "../pursuit/police_chase_x87.hpp"
#include "../../ai/driving/ai_route_planner.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::ai::entities::pending {
using kernel::numeric::Extended;

void calculate_projectile_velocity(void *entity, float *velocity) {
    police_frame::calculate_projectile_velocity_retained(entity, velocity);
}

void begin_projectile_launch_effect(void *projectile) {
    if (!at<Dword>(projectile, 256)) {
        void *helicopter = at<void *>(projectile, 244);
        at<Dword>(projectile, 256) = 5;
        if (void *body = invoke<void *>(interface_at(helicopter, 1, 320), 1))
            play_projectile_impact(at<float>(body, 680), at<float>(body, 684),
                                   at<float>(body, 688), 50, 100);
    }
}

void launch_projectile(void *projectile, void *helicopter, const float *velocity) {
    police_frame::launch_projectile_retained(projectile, helicopter, velocity);
}
// All callers discard the incidental EAX value.
void fire_helicopter_projectiles(void *entity) {
    police_frame::fire_helicopter_projectiles_retained(entity);
}
} // namespace nfs::game::runtime::ai::entities::pending
