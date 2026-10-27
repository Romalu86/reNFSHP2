#pragma once
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/network/session/network_world.hpp"
namespace nfs::game::runtime {
// Integer conversion helpers.
 
int rounded_cell(float);
const float *cross_vectors(const float *, const float *, float *);
const float *cross_vectors_entry(const float *, const float *, float *);
long double distance_squared(const float *, const float *);
void add_vector(const float *, const float *, float *);
extern Dword (&emitter_random_state)[6];
long double turn_sine(float);
long double turn_cosine(float);
void turn_sine_cosine(const float *, float *, float *);
Dword emitter_random_word();
namespace ai {
int kind(void *);
void *racer(void *);
void *police(void *);
float *pair_state(void *, void *);
long double pair_distance(void *, void *);
long double priority(void *);
void rank_racers(Dword *);
void assign_targets(Dword *, Dword);
void advance_pursuit(Dword *);
Dword random_word();
extern Dword random_state[6], random_calls;
void reset_pair_clock(Dword *);
void reset_pursuit_clock(Dword *);
Dword *collision_state();
void clear_collision_state();
Dword *reset_collision_header(Dword *);
void begin_collision_pass(Dword *);
void finish_collision_pass(Dword *);
namespace pending {
extern Dword *const path_controller;
extern int &path_deadline;
void advance_pairs(Dword *);
void update_path_states_entry(Dword *);
Byte spawn_path_vehicle_entry(Dword *, Dword *);
void dispatch_vehicle_event(Dword *, Dword);
void activate_pursuer_entry(void *, Dword);
Byte assign_pursuer_entry(void *, void *);
void update_collision_vehicle_entry(Dword *);
} // namespace pending
} // namespace ai
namespace track {
int next_section(Dword *, int);
int previous_section(Dword *, int);
int previous_pair(Dword *, int);
void *boundary(Dword *, int, int);
void *spatial_cell(Dword *, const float *);
void *polygon_vertices(Dword *, const std::uint16_t *, float **, float **, float **, float **);
Byte contains(Dword *, const float *);
Byte compartment_ready(Dword *);
void open_compartment(Dword *, Dword);
Byte update_direction(Dword *, int);
void direction_changed(Dword *);
void request_next_pair(Dword *, int);
void rotate_compartments(Dword *, int);
void advance_compartment(Dword *, Dword);
namespace pending {
void reset_compartment(Dword *);
void load_compartment_entry(Dword *, Dword);
void bind_compartment(Dword *, Dword);
void advance_replay(Dword *, int);
long double vehicle_direction_entry(void *);
} // namespace pending
} // namespace track
namespace physics {
// Owning footprints and registration writers remain explicit dependencies.
namespace pending {
extern Byte vehicle_flags[];
extern Byte vehicle_states[];
extern int last_vehicle;
extern Dword &collision_disabled;
extern float collision_point[3], collision_normal[3];
extern Byte spatial_storage[98352];
extern Byte (&collision_grid)[64 * 64 * 24];
extern Byte (&global_collision_cell)[24];
extern Byte (&moved_sentinel)[12];
extern Dword *&moved_nodes;
void advance_car(void *);
void advance_body(void *);
Byte collide_shape(void *);
Dword collide_wheels_entry(void *);
int collide_ground(void *);
int collide_pair(void *, void *);
int resolve_pair(void *, void *, const float *, const float *);
long double normalize_collision_entry(float *);
void submit_impact_entry(const float *, Dword, Dword, float, Dword);
void update_contacts_entry(void *, Byte);
void finish_body(void *);
void begin_track_contacts_entry(Dword *);
void update_track_car_entry(void *);
} // namespace pending
void cell_coordinates(const float *, int *, int *);
Byte *global_cell();
Byte *cell(int, int);
void reinsert_moved_nodes();
void *vehicle_car(void *);
Byte is_local_vehicle(void *);
Byte car_is_racer(void *);
Byte car_is_remote(void *);
void *local_car(Dword *, int);
Byte split_screen();
void *reserve_vehicle();
void recompute_last_vehicle();
void release_vehicle(void *);
void reset_vehicle_registry();
void *construct_vehicle(void *);
void destroy_vehicle(void *);
void construct_vehicle_registry();
void destroy_vehicle_registry();
namespace pending {
void reset_vehicle_entry(void *);
}
void project_velocity(void *);
} // namespace physics
namespace race_audio {
int group_volume(Dword, Dword, Dword);
void stop_event(Dword);
void retire_unused_channels();
void play_due_events();
void camera_position(Dword *, Dword *);
void camera_orientation(Dword *, float *);
int set_stream_volume(Dword, Dword);
int fade_stream_volume(Dword, int, Dword);
void *stream(Dword);
namespace pending {
// Pointers to shared audio storage.
extern Byte *const event_flags, *const listener_state;
extern Dword *const race_audio_event_channels;
extern int race_audio_listener_count, &listener_index;
extern Dword &music_mode, &music_level, &music_handle;
extern Dword &car_channel;
extern Dword *listener_configuration;
extern Byte *const delayed_events;
extern Dword &delayed_count;
extern Byte &backend_stream_slot_count;
extern void *stream_slots[];
void set_cd_audio_volume(Dword);
void reset_listener(Dword);
void reset_environment();
void reset_engine_events();
void update_finished_car(void *, Dword);
void set_listener(void *, void *);
void update_car_sounds();
void update_surfaces(Dword);
void update_wheel(void *);
void update_nearest();
void update_emitter(void *);
void update_emitter_channel(void *);
void update_engines();
void update_impacts();
void play_event(Dword, Dword, Dword, Dword, Dword, Dword);
Dword spatial_pan(const void *, const void *);
} // namespace pending
} // namespace race_audio
} // namespace nfs::game::runtime
