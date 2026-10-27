#pragma once
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
extern Dword cop_meter_threshold;
extern float minimum_chase_speed;
extern Dword roadblock_start_level;
extern Dword roadblock_start_strategy;
extern float spike_strip_size;
extern float roadblock_spacing;
extern float roadblock_evade_fraction;
extern float player_reset_cop_distance;
extern float player_reset_spike_distance;
extern Dword finish_immunity_seconds;
extern float minimum_detection_radius, speeder_speed_mph, reverse_speed_mph;
extern float projectile_drop_height;
extern float projectile_drop_back;
extern float projectile_mass;
extern float projectile_elasticity;
extern Dword minimum_cop_stun_ticks;
extern Byte cop_can_be_stunned;
extern Byte traffic_can_stun_cop;
extern Byte racer_can_stun_cop;
extern float cop_stun_radius;
extern Dword pursuit_level_seconds[5];
extern Dword chase_warmup_ticks, helicopter_activation_ticks;
void set_pursuit_level_duration(Dword, Dword);
} // namespace nfs::game::runtime::ai::lifecycle::pending
