#pragma once
#include <cstdint>
#if defined(__clang__) && __LDBL_MANT_DIG__ == 64
#include "../../math/vectors/frame_x87_private.hpp"
#include "ai_path_frame_x87.hpp"
#endif
namespace nfs::game::runtime::ai::cars::drive_frame {
void traffic_controls(void *, std::uint32_t);
void route_heading_native80(void *, void *);
void player_target_speed_native80(void *, float, void *);
float player_target_speed_f32(void *, float);
std::uint8_t traffic_ahead(void *);
std::uint8_t pursuit_controls(void *, std::uint32_t *, float);
void update_cop_controls(void *, std::uint32_t);
void racer_body_contact(void *, std::uint32_t);
std::uint32_t accept_player_side_contact(void *, std::uint32_t, std::uint32_t);
void player_before_start(void *, std::uint32_t);
void player_start_path(void *, std::uint32_t);
void player_path(void *, std::uint32_t);
void toggle_pursuit(void *);
namespace legacy {
int corridor_lane_native80(const void *);
float compatibility_lane_floor(float shifted);
bool route_heading_below_half(void *);
bool lateral_beyond_three_quarters(void *);
bool route_heading_negative(void *);
bool lateral_negative_for_target(void *, void *);
float lateral_f32(void *);
}
#if defined(__clang__) && __LDBL_MANT_DIG__ == 64
inline long double number(float value) { return static_cast<long double>(value); }
inline long double integer(std::int64_t value) { return static_cast<long double>(value); }
inline long double magnitude(long double value) { return value < 0.0L ? -value : value; }
inline long double point_distance(const float *first, const float *second) {
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(static_cast<long double>(second[axis]) - first[axis]);
    return hardware::vector_length(delta);
}
inline void discard_steering(void *entity, const float *position, void *controls) {
    unsigned char retained[10];
    path_frame::steering_native80(entity, position, controls, retained);
}
#endif

}
