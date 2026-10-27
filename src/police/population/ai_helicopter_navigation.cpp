#include "../pursuit/police_chase_x87.hpp"
#include "../../ai/driving/ai_route_planner.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::entities::pending {
using kernel::numeric::Extended;




void steer_helicopter_patrol(void *entity) {
    police_frame::steer_helicopter_patrol_retained(entity);
}

void steer_helicopter_pursuit(void *entity) {
    police_frame::steer_helicopter_pursuit_retained(entity);
}
} // namespace nfs::game::runtime::ai::entities::pending
