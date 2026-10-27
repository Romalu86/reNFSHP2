#pragma once

namespace nfs::game::runtime::physics::contacts_pending::x87 {
void wheel_slip_strength_native80(void *car, int wheel, void *output);
}

namespace nfs::game::runtime::track::x87 {
void vehicle_direction_native80(void *vehicle, void *output);
}

#ifdef __clang__
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>

namespace nfs::game::runtime::physics::motion_x87 {
inline long double retained(float value) { return static_cast<long double>(value); }
inline void store_native80(long double value, void *output) {
    std::memcpy(output, &value, 10);
}
} // namespace nfs::game::runtime::physics::motion_x87
#endif
