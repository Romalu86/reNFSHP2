#pragma once
#include "ai_car_runtime.hpp"
#include "ai_route_planner.hpp"
namespace nfs::game::runtime::ai::cars {
void *autonomous_cop_entity(void *);
void *traffic_entity(void *);
void *spike_entity(void *);
void *mutable_spike_entity(void *);
void *projectile_entity(void *);
int contact_entity_count(void *);
Dword car_is_traffic(void *);
void clear_path_history(void *);
float path_side_width(void *, const void *);
Byte recently_stunned(void *, Byte);
kernel::numeric::Extended steer_car_to_position(void *, const float *, void *);
void puncture_wheel(void *, Byte);
Dword spike_wheel_hit_mask(void *, void *);
Byte apply_spike_strip_hits(void *);
struct ContactSoundVector {
    float x, y, z;
};
static_assert(sizeof(ContactSoundVector) == 12);
void play_car_contact_sound(Dword, Dword, Dword, Dword, Dword, Dword, ContactSoundVector,
                            ContactSoundVector);
Dword participant_completed_checkpoints(void *, Dword);
Byte camera_mode_flashes(void *);
Dword should_draw_network_car(Dword);
kernel::numeric::Extended distance_to_local_players(const float *);
float local_player_distance(const float *);
void record_car_controls(void *, Dword, const Dword *);
Byte replay_car_controls(void *, Dword, Dword *);
void collect_car_controls(void *, Dword, Dword *);
void read_car_action_bindings(void *, Dword,
                              Dword *);
Byte target_on_neighboring_section(void *, void *);
} // namespace nfs::game::runtime::ai::cars
