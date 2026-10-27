#pragma once

namespace nfs::game::runtime::hardware {
int current_integer_f32(float value);
long long truncate_integer_native80(const void *value);
}

namespace nfs::game::runtime {
long double turn_tangent_retained(float angle);
long double turn_direction_retained(float x, float z);
long double actor_sine_retained(float radians);
long double actor_cosine_retained(float radians);
long double network_rotation_angle_retained(float cosine);
}

namespace nfs::game::runtime::ai::entities::pending::direction_x87 {
void direction_turns_native80(float x, float z, void *output);
void steer_toward_position_native80(void *entity, const float *target, void *controls,
                                  void *output);
}

namespace nfs::game::runtime::ai::pending {

void store_pair_distances(float *pair, const float *position, const float *other_position);
}

namespace nfs::game::runtime::physics::car_frame::legacy {
// Discard compatibility aggregates within MSVC; no Extended object crosses ABI.
void discard_rolling_resistance(void *car);
void discard_clutch_limit();
}

namespace nfs::game::runtime::physics::legacy {
bool local_player_near_150(const float *position);
void tire_lateral_f32(void *tire, void *wheel, float *value);
void tire_aligning_f32(void *tire, void *wheel, float *value);
void engine_torque_f32(void *engine, int gear, float *value);
int speed_to_rpm(float speed);
int round_road_rpm(float speed);
long double power_retained(double base, double exponent);
}

namespace nfs::game::runtime::physics::car_frame::x87 {
void wheel_force_scale_native80(void *car, void *value);
void rolling_resistance_native80(void *car, void *value);
}
