#include "ai_chase_tuning.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
Dword cop_meter_threshold = 50;
float minimum_chase_speed = 30.0f;
Dword roadblock_start_level = 4294967295u;
Dword roadblock_start_strategy = 4294967295u;
float spike_strip_size = 1.0f;
float roadblock_spacing = 0.20000000298023223876953125f;
float roadblock_evade_fraction = 0.5f;
float player_reset_cop_distance = 10.0f;
float player_reset_spike_distance = 30.0f;
Dword finish_immunity_seconds = 20;
float minimum_detection_radius = 40.f;
float speeder_speed_mph = 60.f;
float reverse_speed_mph = -15.f;
float projectile_drop_height = 25.0f;
float projectile_drop_back = 1.0f;
float projectile_mass = 250.0f;
float projectile_elasticity = 1.5f;
Dword minimum_cop_stun_ticks = 1;
Byte cop_can_be_stunned = 1;
Byte traffic_can_stun_cop = 1;
Byte racer_can_stun_cop = 1;
float cop_stun_radius = 50.0f;
Dword pursuit_level_seconds[5] = {30, 30, 45, 45, 45};
Dword helicopter_activation_ticks{};
} // namespace nfs::game::runtime::ai::lifecycle::pending
