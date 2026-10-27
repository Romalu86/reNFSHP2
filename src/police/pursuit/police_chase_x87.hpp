#pragma once
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#if defined(__clang__) && __LDBL_MANT_DIG__ == 64
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../ai/driving/ai_path_frame_x87.hpp"
#endif
namespace nfs::game::runtime::ai::police_math {
std::uint32_t truncate_native80(const void *);
float local_player_distance(const float *);
bool heading_allows_relocation(void *);
bool lateral_above_three_quarters(void *);
#if defined(__clang__) && __LDBL_MANT_DIG__ == 64
inline long double number(float value) { return static_cast<long double>(value); }
inline std::uint32_t truncate(long double value) { return truncate_native80(&value); }
inline long double magnitude(long double value) { return value < 0.0L ? -value : value; }
inline long double point_distance(const float *first, const float *second) {
    float delta[3];
    for (unsigned axis = 0; axis != 3; ++axis)
        delta[axis] = float(static_cast<long double>(second[axis]) - first[axis]);
    return hardware::vector_length(delta);
}
inline long double steer_car(void *entity, const float *point, void *controls) {
    long double value;
    cars::path_frame::steering_native80(entity, point, controls, &value);
    return value;
}
inline long double steer_entity(void *entity, const float *point, void *controls) {
    long double value;
    entities::pending::direction_x87::steer_toward_position_native80(entity, point, controls, &value);
    return value;
}
#endif
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
std::uint32_t chase_ticks(float seconds);
float evade_fraction(float percentage);
}

namespace nfs::game::runtime::ai::lifecycle::pending::police_frame {
void *allocate_police_chase_retained(void *owner, void *cop, void *target, Byte forced_announcement);
void begin_police_chase_retained(void *chase, void *cop, void *target, Dword level);
void update_police_chase_retained(Dword *owner, void *chase);
Byte needs_chase_backup_retained(void *chase);
Byte bust_timer_expired_retained(void *perp);
Dword cop_threat_zone_retained(void *target, void *cop);
Dword place_roadblock_cop_retained(void *location, void *cop);
void place_roadblock_spikes_retained(void *location, void *spike);
Dword sound_location_retained(const float *position);
void update_chase_announcements_retained(void *owner, void *chase);
void update_helicopter_announcement_retained(void *owner, void *chase);
void activate_chase_helicopter_retained(void *helicopter, void *chase);
Byte find_pursuit_spawn_retained(void *entity, Byte reverse, float minimum_distance, float player_range,
                        Dword *section_output, int *node_output);
} // namespace nfs::game::runtime::ai::lifecycle::pending::police_frame

namespace nfs::game::runtime::ai::cars::police_frame {
Byte request_network_spikes_retained(void *entity);
Byte detect_speeding_racers_retained(void *entity);
void update_cop_patrol_target_retained(void *entity, Dword controls);
void __fastcall autonomous_cop_recover_body_retained(void *entity, void *, Dword controls_word);
Byte relocate_distant_chase_cop_retained(void *entity);
void __fastcall ai_cop_frame_retained(void *entity, void *);
Dword __fastcall player_pursuit_event_retained(void *entity, void *, Dword controls_word);
void __fastcall player_path_brake_retained(void *entity, void *, Dword controls_word);
Dword spike_wheel_hit_mask_retained(void *spike, void *entity);
} // namespace nfs::game::runtime::ai::cars::police_frame

namespace nfs::game::runtime::ai::lifecycle::police_frame {
Byte assign_pursuer_retained(void *entity, void *target);
} // namespace nfs::game::runtime::ai::lifecycle::police_frame

namespace nfs::game::runtime::ai::entities::police_frame {
void __fastcall set_helicopter_speed_retained(void *entity, void *, float desired, Dword control_address);
Byte settle_spike_body_retained(void *entity);
} // namespace nfs::game::runtime::ai::entities::police_frame

namespace nfs::game::runtime::ai::entities::pending::police_frame {
void calculate_projectile_velocity_retained(void *entity, float *velocity);
void launch_projectile_retained(void *projectile, void *helicopter, const float *velocity);
void fire_helicopter_projectiles_retained(void *entity);
void steer_helicopter_patrol_retained(void *entity);
void steer_helicopter_pursuit_retained(void *entity);
void apply_helicopter_controls_retained(void *entity, float desired_speed, const float *target);
void apply_projectile_pursuit_damage_retained(void *cop, float damage);
void play_projectile_impact_retained(float x, float y, float z, Dword event, int maximum_distance);
} // namespace nfs::game::runtime::ai::entities::pending::police_frame
