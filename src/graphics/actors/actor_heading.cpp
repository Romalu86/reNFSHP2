#include "actor_pose_math.hpp"
#include <cmath>
#include <cstring>

#pragma intrinsic(atan2)

namespace nfs::game::world::actor_resources::pending {

// This MSVC intrinsic leaf consumes the captured float representations directly.
// Its float result is the original stored-radians boundary.
float actor_heading_radians(std::uint32_t x_bits, std::uint32_t z_bits) {
    float x;
    float z;
    std::memcpy(&x, &x_bits, sizeof(x));
    std::memcpy(&z, &z_bits, sizeof(z));
    return static_cast<float>(std::atan2(static_cast<double>(x), static_cast<double>(z)));
}

} // namespace nfs::game::world::actor_resources::pending
