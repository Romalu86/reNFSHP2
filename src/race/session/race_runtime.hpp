#pragma once
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/network/session/network_world.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include <initializer_list>
namespace nfs::network {
Dword race_pending(Dword *);
}
namespace nfs::application::pending {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
using RaceMethod = void(__cdecl *)();
extern const RaceMethod race_state_methods[48], event_base_methods[48], paired_event_methods[48];
extern const RaceMethod event_zero_methods[48], event_one_methods[48], event_two_methods[48];
extern const RaceMethod event_three_methods[48], event_four_methods[48], event_five_methods[48];
extern const RaceMethod event_six_methods[48], event_seven_methods[48], event_nine_methods[48];
extern const RaceMethod event_eleven_methods[48], event_twelve_methods[48],
    event_thirteen_methods[48];
struct RaceParticipant {
    const Dword *methods;
    int identifier, best_lap_ticks, last_checkpoint, completed_checkpoints, awarded_medal;
    Dword reserved_18;
    Dword takedowns, arrests, losses, reserved_28;
    float maximum_speed, average_speed;
    int checkpoint_time;
    int checkpoint_times[100], checkpoint_positions[100];
    Dword reserved_358;
    gui::String name;
    int finish_position, retirement_tick, best_time;
    Dword counters[10], checkpoint_stamp;
    int finish_time;
    Byte disabled, retired, disconnected, reserved_3af;
    int kind;
    Dword awards[5], award_times[5];
};
static_assert(sizeof(RaceParticipant) == 988 && offsetof(RaceParticipant, name) == 860);
static_assert(offsetof(RaceParticipant, best_lap_ticks) == 8 &&
              offsetof(RaceParticipant, last_checkpoint) == 12 &&
              offsetof(RaceParticipant, completed_checkpoints) == 16 &&
              offsetof(RaceParticipant, maximum_speed) == 44 &&
              offsetof(RaceParticipant, average_speed) == 48);
static_assert(offsetof(RaceParticipant, kind) == 944 &&
              offsetof(RaceParticipant, finish_time) == 936);
inline RaceParticipant *participant(void *race, Dword index) {
    return reinterpret_cast<RaceParticipant *>(bytes(race) + 584 + index * 988);
}
inline Dword race_clock() {
    return graphics::unresolved::camera_state;
}
void *construct_race_state(void *, Dword, Dword);
void destroy_race_state(void *);
void *copy_race_state(void *, const void *);
void *copy_construct_race_state(void *, const void *);
void *construct_event_base(void *, Dword, Dword);
void *construct_paired_event(void *, Dword, Dword);
void *construct_event_zero(void *, Dword, Dword);
void *construct_event_one(void *, Dword, Dword);
void *construct_event_three(void *, Dword, Dword);
void *construct_event_four(void *, Dword, Dword);
void *construct_event_five(void *, Dword, Dword);
void *construct_event_six(void *, Dword, Dword);
void *construct_event_nine(void *, Dword, Dword);
void *construct_event_eleven(void *, Dword, Dword);
void *construct_event_twelve(void *, Dword, Dword);
void *construct_event_thirteen(void *, Dword, Dword);
void initialize_race_rewards(void *);
void reset_race_rewards(void *);
void load_race_reward_rules(void *, const void *);
void read_race_reward_rules(void *, void *, void *);
void *active_event_progress(void *);
void mark_event_entered(void *);
void *construct_participant(void *);
void reset_participant(void *);
void *assign_participant(void *, const void *);
void record_takedown(void *);
void record_arrest(void *);
inline Byte participant_has_finished(void *record) {
    return Byte(network::race_pending(static_cast<Dword *>(record)));
}
Byte participant_completed_checkpoint(void *, int);
int participant_checkpoint_duration(void *, int);
Byte participant_is_cop(void *, Dword);
Byte participant_blocked(void *, Dword);
inline void set_participant_blocked(void *race, Dword slot, Byte value) {
    game::runtime::ai::dependencies::set_participant_enabled(race, slot, value);
}
Byte participant_disconnected(void *, Dword);
Byte deadline_expired(void *);
Byte bonus_display_enabled(void *);
int count_racers_ahead(void *, int);
void *copy_event_base(void *, const void *);
void *copy_paired_event(void *, const void *);
void finish_participant(void *, Dword);
void __fastcall record_checkpoint(void *, void *, Dword, int, Dword);
void __fastcall record_collision(void *, void *, Dword, Dword, Dword);
void add_participant_reward(void *, Dword, Dword);
Dword collision_reward(void *, int, int);
Dword takedown_reward(void *, int);
void __fastcall reset_race_statistics(void *, void *);
Byte __fastcall event_complete(void *, void *);
} // namespace nfs::application::pending
