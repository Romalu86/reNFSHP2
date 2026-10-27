#pragma once
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstdint>

namespace nfs::game::runtime::physics::car_frame::drive {
extern const float handbrake_tick_scale, handbrake_half32, handbrake_exponent_scale;
extern const double handbrake_one64, handbrake_half64;
long double handbrake_blend_retained(int ticks);
void control_clamp_f32(float value, float minimum, float maximum, float *output);
void transmission_clamp_f32(float value, float minimum, float maximum, float *output);
std::int32_t truncate_engine_rpm(const void *retained_rpm);
}

#ifdef __clang__
#include "car_curve_x87.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstdint>
#include <cstring>

namespace nfs::game::runtime::physics::car_frame::drive {
inline long double retained(float value) { return static_cast<long double>(value); }
inline long double integer(std::int64_t value) { return static_cast<long double>(value); }
inline long double absolute(long double value) { return value < 0.0L ? -value : value; }
inline long double sine(float turns) { return runtime::turn_sine(turns); }
inline long double cosine(float turns) { return runtime::turn_cosine(turns); }
inline long double direction(float x, float z) { return runtime::turn_direction_retained(x, z); }

inline long double load_native80(const void *bytes) {
    long double value;
    std::memcpy(&value, bytes, 10);
    return value;
}
inline long double steering_input(void *car) {
    unsigned char value[10];
    curves_x87::car_steering_input_native80(car, value);
    return load_native80(value);
}
inline long double throttle_curve(void *car) {
    unsigned char value[10];
    curves_x87::car_throttle_curve_native80(car, value);
    return load_native80(value);
}
inline long double brake_pressure(float force, std::uint32_t flags, int kind, float amount) {
    unsigned char value[10];
    world::vehicle_setup::curves_x87::brake_torque_native80(force, flags, kind, amount, value);
    return load_native80(value);
}
inline float control_clamp(float value, float minimum, float maximum) {
    float output;
    control_clamp_f32(value, minimum, maximum, &output);
    return output;
}
inline float transmission_clamp(float value, float minimum, float maximum) {
    float output;
    transmission_clamp_f32(value, minimum, maximum, &output);
    return output;
}
} // namespace nfs::game::runtime::physics::car_frame::drive
#endif
