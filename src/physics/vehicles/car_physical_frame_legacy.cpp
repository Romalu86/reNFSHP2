#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>

namespace nfs::game::runtime::physics::car_frame::drive {

extern const float handbrake_tick_scale = 0.0333333350718021392822265625f;
extern const float handbrake_half32 = .5f;
extern const float handbrake_exponent_scale = -12.56637096405029296875f;
extern const double handbrake_one64 = 1.;
extern const double handbrake_half64 = .5;

// Keep the inherited __ftol conversion boundary within the MSVC ABI.
std::int32_t truncate_engine_rpm(const void *retained_rpm) {
    const auto low_word = static_cast<std::uint32_t>(
        hardware::truncate_integer_native80(retained_rpm));
    std::int32_t result;
    std::memcpy(&result, &low_word, sizeof(result));
    return result;
}
} // namespace nfs::game::runtime::physics::car_frame::drive
