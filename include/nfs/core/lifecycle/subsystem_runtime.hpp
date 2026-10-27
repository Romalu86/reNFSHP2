#pragma once
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::game::runtime {
using kernel::Dword;
using Byte = std::uint8_t;
using network::packets::at;
using network::packets::bytes;
using network::packets::system;
template <class T> const T &at(const void *object, unsigned offset) {
    return *reinterpret_cast<const T *>(static_cast<const Byte *>(object) + offset);
}
inline const Byte *bytes(const void *object) {
    return static_cast<const Byte *>(object);
}
template <class Result = void, class... Args>
Result invoke(void *object, unsigned slot, Args... args) {
    using Method = Result(__thiscall *)(void *, Args...);
    return (*reinterpret_cast<const Method **>(object))[slot](object, args...);
}
extern Dword *step_lists[100];
extern int step_list_count;
extern Dword countdown_delay, started_frames;
extern Byte race_started, stepping, reset_requested, reset_complete;
extern Byte &race_waiting;
extern Dword *track_collision;
extern float &network_step_scale;
extern Dword race_rule_cache[10];
extern Dword &cached_rule_7, &cached_rule_8, &cached_rule_3;
void cache_rule(Dword *, Dword, Dword);
void clear_step_lists();
void add_step_list(Dword *);
void remove_step_list(Dword *);
void reset_step_state();
namespace events {
void acquire(Dword *);
void release(Dword *);
void reset_references(Dword *);
void clear(Dword *);
void dispatch(Dword *);
void remove_at(Dword *, int);
void subscribe(Dword *, int, Dword *);
void unsubscribe(Dword *, Dword *);
void unsubscribe_kind(Dword *, int, Dword *);
void clear_subscribers(Dword *);
} // namespace events
namespace pending {
void advance_track(Dword *, Dword);
void advance_replay_track_entry(Dword *, Dword);
void update_network_entry(Dword *);
void prepare_ai(Dword *);
void finish_ai(Dword *);
void prepare_vehicle_states();
void prepare_cars(Dword *);
void finish_cars(Dword *);
void update_ai_contacts(Dword *, Dword);
void step_vehicle_physics(Dword);
void update_track_collision(Dword *);
void update_race_audio();
void update_car_environment();
void update_particles(Dword *, Dword);
void prepare_start_line_entry(Dword *);
void prepare_start_line_object_entry(Dword *);
void update_start_line_geometry(Dword *);
void update_ai_paths(Dword *);
void update_ai_racers(Dword *);
void prepare_ai_constraints(Dword *);
void update_ai_pursuit(Dword *);
void update_ai_targets(Dword *, Dword);
void update_world_particles(Dword *, Dword);
void update_car_particles(Dword *);
void prepare_vehicle(void *);
} // namespace pending
} // namespace nfs::game::runtime
