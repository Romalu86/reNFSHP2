#pragma once
#include "../../ai/driving/ai_car_behavior.hpp"
#include "../../ai/entities/ai_entity_runtime.hpp"
#include "../../vehicles/setup/car_initialization_runtime.hpp"
#include "../../graphics/vehicles/car_wheel_effects_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include <cmath>
namespace nfs::game::runtime::physics::car_frame {
namespace entities = ai::entities;
using kernel::numeric::Extended;
inline Extended number(float value) {
    return Extended(value, kernel::numeric::current_format());
}
inline Extended integer(std::int64_t value) {
    return Extended::integer(value, kernel::numeric::current_format());
}
inline Extended magnitude(Extended value) {
    return value < number(0) ? -value : value;
}
inline int truncate(Extended value) {
    return int(Dword(value.truncate_to_integer()));
}
inline float *vector_at(void *object, unsigned offset) {
    return reinterpret_cast<float *>(bytes(object) + offset);
}
inline void *wheel_at(void *car, unsigned index) {
    return bytes(car) + 1948 + index * 296;
}
extern float gravity;
extern float inverse_gravity;
inline Extended sine(float turns) {
    return Extended::from_double(double(turn_sine(turns)), kernel::numeric::current_format());
}
inline Extended cosine(float turns) {
    return Extended::from_double(double(turn_cosine(turns)), kernel::numeric::current_format());
}
Extended clamp_control_factor(float, float, float);
Extended clamp_transmission_factor(float, float, float);
Extended car_clutch_limit();
Extended car_throttle_curve(void *);
Extended car_steering_input(void *);
Extended turn_tangent(float);
void announce_gear_selection(int);
void select_reverse_braking(void *);
void select_automatic_gear_by_force(void *);
void update_automatic_gear_selection(void *);
void update_manual_gear_selection(void *);
void update_car_throttle_and_brakes(void *);
void set_wheel_steering(void *, float);
void update_car_transmission(void *);
void integrate_engine_speed(void *);
void distribute_drive_torque(void *);
void apply_drive_torque(void *);
void update_active_car_physics(void *);
void update_kinematic_car_physics(void *);
void sample_wheel_ground(void *);
void solve_wheel_suspension(void *);
void solve_car_tyre_forces(void *);
void update_car_orientation_frame(void *);
void integrate_car_linear_forces(void *);
void update_wheel_surface_motion(void *);
void project_wheel_ground_speeds(void *);
void apply_airborne_resistance(void *);
void apply_body_air_resistance(void *);
void apply_aerodynamic_downforce(void *);
void apply_aerodynamic_drag(void *);
Byte wheel_reverse_brake(void *);
void apply_car_brakes(void *);
void apply_wheel_brake_torque(void *, float, int);
Extended apply_wheel_rolling_resistance(void *);
void apply_lateral_weight_transfer(void *);
void apply_body_roll_weight_transfer(void *);
void solve_low_speed_wheel_forces(void *);
void limit_wheel_friction_forces(void *);
void integrate_wheel_angular_speed(void *);
void accumulate_car_torques(void *);
Extended wheel_force_scale(void *);
void sample_wheel_slip_angle(void *);
void solve_wheel_lateral_force(void *);
void update_kinematic_engine_rpm(void *);
void update_car_track_sample(void *);
void update_car_statistics(void *);
void record_quarter_mile(void *);
void record_four_hundred_metres(void *);
void record_sixty_mph(void *);
void record_hundred_mph(void *);
void record_braking_distance(void *, int);
} // namespace nfs::game::runtime::physics::car_frame
