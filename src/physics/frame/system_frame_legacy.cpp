#include "../vehicles/car_physics_frame.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"

namespace nfs::game::runtime::physics::legacy {
bool local_player_near_150(const float *position) {
    return ai::cars::local_player_distance(position) < 150.f;
}
} // namespace nfs::game::runtime::physics::legacy
