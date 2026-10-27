#pragma once
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"

namespace nfs::game::runtime::ai::lifecycle {
// Contiguous globals. This is a byte footprint, not an inferred C++ object.
extern Byte storage[916];
template <class T> T &state(unsigned address) {
    return *reinterpret_cast<T *>(storage + (address - 7352412));
}
void initialize(Dword *);
void shutdown(Dword *);
void load_level(Dword *);
void unload_level(Dword *);
void reset_level(Dword *);
Dword *construct_entity_list(Dword *);
void destroy_entity_list(Dword *);
void clear_entity_list(Dword *);
void reset_pursuit_schedule(Dword *);
void clear_pursuit_schedule(Dword *);
void begin_path_level();
void end_path_level();
int selectable_car_count(Dword *);
int selectable_car(Dword *, int);
Dword car_variant_count(Dword *, int);
void dispatch_cop_event(Dword *, Dword);
void deactivate_car_dynamics(Dword *);
void reset_chase_contact(void *);
void activate_pursuer(void *, Dword);
void update_path_states(Dword *);
Byte spawn_path_vehicle(Dword *, Dword *);
Byte reset_cop_vehicle(Dword *, Dword *);
Byte player_within_range(Dword *, const float *, float);
Byte retain_path_vehicle(void *);
Byte release_path_reservation(void *, void *);
Byte assign_pursuer(void *, void *);
void update_collision_vehicle(Dword *);
int path_length(void *, int);
void locate_path_sample(void *, int, void **, int *);
void *path_section_at(void *, int);
void sample_path(void *, int, float *);
void *path_current_section(void *);
void *reset_path_reference(void *);
void *copy_path_reference(void *, const void *);
void clear_path(void *);
void reverse_path(void *);
void bind_path(void *, const void *, Dword, Dword, void *);
void choose_least_used_section(void *);
void *reverse_path_reference(void *);
long double random_fraction();
void rotate_y_basis(const float *, float, float *);
namespace pending {
void read_police_chase_tuning();
int load_racer_profiles(Dword *);
Dword *construct_net_profile(Dword *, int);
extern const std::int32_t net_profile_base_offsets[];
extern const Dword profile_base_methods[];
extern const Dword net_profile_methods[];
void read_race_tuning(Dword *);
void initialize_path_population(Dword *);
void reset_path_population(Dword *);
void release_path_population(Dword *);
int load_helicopter_population(Dword *, int);
void finish_population_load(Dword *);
void reset_path_controller(Dword *);
void anchor_paths(Dword *);
void recount_entity_kinds(Dword *);
int parse_car_kind(const char *);
int parse_car_variant(int);
int parse_car_skin(const char *);
Byte reset_cop(Dword *, Dword *);
extern float spawn_distance;
extern float helicopter_rotor_increment;
extern Dword fractional_random_calls;
void sample_section_pose(void *, int, float *);
void *enumerate_path_sections(void *, Dword);
void update_path_branch(void *, void *);
void construct_pursuit_route(void *, void *, Dword *, Dword *, const float *, const float *, void *,
                             Dword, Dword);
void set_car_speed(void *, float);
void synchronize_track_vehicle(void *);
void update_helicopter_attachments(void *);
void update_helicopter_rendering(void *);
void position_helicopter_sound(const float *, const float *);
Byte find_pursuit_spawn(void *, Byte, float, float, Dword *,
                        int *);
void sample_route_pose(void *, int, float *, float *,
                       Byte);
void position_entity(void *, const float *, const float *, Byte);
Byte advance_chase_warmup(void *);
void update_police_chase(Dword *, void *);
void complete_police_chase(Dword *, void *, int);
Byte participant_enabled(void *, Dword);
void set_participant_enabled(void *, Dword, int);
void clear_car_contact(Dword *, int);
void finalize_network_load(Dword *);
void spawn_racer(Dword *, Dword, int, Dword, Dword);
void spawn_player(Dword *, Dword, Dword, Dword);
void spawn_cop(Dword *, Dword, Dword, Dword, Byte, Byte);
void spawn_remote_car(Dword *, Dword, int, Dword, int, Byte);
Dword *construct_traffic_car(Dword *, Dword, int);
void attach_pursuit_car(Dword *, Dword *);
Dword select_car_kind(int);
Dword car_kind_name(Dword);
extern Dword &disable_ai;
extern Dword &explicit_car_kind;
extern char (&car_kind_text)[24];
extern Dword &explicit_car_skin;
extern char (&car_skin_text)[24];
extern Dword &explicit_car_variant;
extern char (&car_variant_text)[24];
extern Byte population_loading;
extern Dword startup_ticks;
extern Dword load_tick_4, load_tick_10, load_tick_end;
} // namespace pending
} // namespace nfs::game::runtime::ai::lifecycle


