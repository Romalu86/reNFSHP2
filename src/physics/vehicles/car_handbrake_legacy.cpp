#include "car_drive_frame_x87.hpp"
#include <cmath>

#pragma intrinsic(exp)

namespace nfs::game::runtime::physics::car_frame::drive {

long double handbrake_blend_retained(int ticks) {
    const auto exponent =
        (static_cast<long double>(ticks) * handbrake_tick_scale - handbrake_half32) *
        handbrake_exponent_scale;
    return handbrake_half64 / (std::exp(exponent) + handbrake_one64) + handbrake_half64;
}
} // namespace nfs::game::runtime::physics::car_frame::drive
