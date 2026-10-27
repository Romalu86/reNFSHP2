#pragma once
#include "../../ai/driving/ai_route_planner.hpp"
#include "../../network/vehicles/remote_car_runtime.hpp"
#include "nfs/audio/speech/speech_game.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../../ai/population/ai_population_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::entities::pending {
Dword set_police_chase_outcome(void *, Dword);
}
namespace nfs::application::pending {
void *traffic_actor_view(void *);
}
namespace nfs::game::runtime::ai::lifecycle::pending {
inline float saved_float(Dword word) {
    float value;
    std::memcpy(&value, &word, 4);
    return value;
}
extern Dword threat_memory_ticks;
Dword chase_elapsed_ticks(void *);
Byte needs_chase_backup(void *);
Dword update_chase_meter(void *, std::int16_t);
void update_bust_timer(void *, float, Byte);
Byte update_bust_speed(void *, float);
Byte bust_timer_expired(void *);
Dword cop_threat_zone(void *, void *);
Byte attach_chase_cop(void *, void *);
Byte request_chase_cop(void *, void *, Dword);
Byte __fastcall request_backup_car(void *, void *);
void position_chase_cops(void *, Dword, int);
void clear_police_chase(void *);
void finish_chase(void *, void *);
Byte request_roadblock(void *, void *);
Dword place_roadblock_cop(void *, void *);
void place_roadblock_spikes(void *, void *);
void position_roadblock_spikes(void *, const float *, const float *);
void *reserve_spike_entity(void *);
Byte passed_roadblock(void *, void *);
void update_spike_announcement(void *, void *);
void update_chase_helicopter(void *, void *);
void bind_chase_helicopter(void *, void *);
void *reserve_chase_helicopter(void *);
void activate_chase_helicopter(void *, void *);
void reset_helicopter_position(void *, const float *);
int pursuit_listener(void *);
int player_chase_listener(void *);
int player_target_listener(void *);
Dword sound_location(const float *);
void update_chase_announcements(void *, void *);
void update_helicopter_announcement(void *, void *);
Dword *sound_location_marker(void *, int);
Byte record_support_request(void *, Dword, int);
void release_support_request(void *, Dword, int);
void decrement_support_statistic(void *, int);
Byte increment_support_statistic(void *, int);
Dword police_colour_mask(int);
void select_police_voice(void *, Byte, Dword);
Dword chase_active(void *);
void begin_police_chase(void *, void *, void *, Dword);
void choose_chase_level_three(void *, void *, void *);
void choose_chase_level_four(void *, void *, void *);
void choose_chase_level_five(void *, void *, void *);
void *allocate_police_chase(void *, void *, void *, Byte);
Dword support_statistic_limit(void *, Dword);
inline void *body_interface(void *entity) {
    return entities::interface_at(entity, 3);
}
inline void *perp_interface(void *entity) {
    return entities::interface_at(entity, 4);
}
inline void *entity_body(void *entity) {
    return invoke<void *>(body_interface(entity), 1);
}
inline Dword now() {
    return graphics::unresolved::camera_state;
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
