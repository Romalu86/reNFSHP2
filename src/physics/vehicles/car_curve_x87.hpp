#pragma once
#include <cstdint>
namespace nfs::game::world::vehicle_setup::curves_x87 {
void engine_torque_native80(void *, int, void *);
void brake_torque_native80(float, std::uint32_t, int, float, void *);
void tire_lateral_force_native80(void *, void *, void *);
void tire_aligning_moment_native80(void *, void *, void *);
void rolling_resistance_native80(float, void *);
}
namespace nfs::game::world::vehicle_setup::legacy {
int slip_interval_f32(float);
}
namespace nfs::game::runtime::physics::car_frame::curves_x87 {
void car_throttle_curve_native80(void *, void *);
void car_steering_input_native80(void *, void *);
void turn_tangent_native80(float, void *);
}
