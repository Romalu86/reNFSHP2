#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace nfs::game::world {
using kernel::Dword;
using Byte = std::uint8_t;
using runtime::at;
using runtime::bytes;
using runtime::invoke;
inline void *subsystem(unsigned offset) {
    return lifecycle::manager->slots[offset / 4];
}
inline Byte *offset(void *p, unsigned n) {
    return static_cast<Byte *>(p) + n;
}
void enter_vehicle_frontend(void *);
void leave_vehicle_frontend(void *);
void load_vehicle_level(void *);
void unload_vehicle_level(void *);
void reset_vehicle_level(void *);
void initialize_vehicle_catalog(void *);
void initialize_track_catalog(void *);
void load_track_manager_level(void *);
void unload_track_manager_level(void *);
void load_level_resources(void *);
void unload_level_resources(void *);
void load_track_level(void *);
void unload_track_level(void *);
void reset_track_level(void *);
void load_car_level(void *);
void unload_car_level(void *);
void reset_car_level(void *);
void enter_car_frontend(void *);
void enter_camera_frontend(void *);
void load_camera_level(void *);
void unload_camera_level(void *);
void reset_camera_level(void *);
void *construct_camera_director(void *, int);
void *construct_camera_modes(void *);
void allocate_camera_modes(void *, int);
void *construct_camera_mode_array(void *, int);
void destroy_camera_modes(void *);
void *delete_camera_modes(void *, Dword);
void destroy_camera_director(void *);
void append_camera_mode(void *, void *);
void *detach_camera_mode(void *, Dword);
void *find_camera_mode(void *, Dword);
void end_camera_controller(void *, void *);
void end_all_camera_modes(void *);
void bind_camera_player(void *, int);
void notify_camera_mode(void *);
void begin_camera_mode(void *, Dword);
void end_camera_mode(void *, Dword);
void notify_camera_listeners(void *);
void initialize_characters(void *);
void shutdown_characters(void *);
void load_character_level(void *);
void unload_character_level(void *);
void reset_character_level(void *);
void enter_character_frontend(void *);
void leave_character_frontend(void *);
void load_start_line(void *);
void prepare_start_line(void *);
void prepare_start_line_object(void *);
void quaternion_matrix(const float *, float *);
int compartment_ready(void *, int);
void advance_replay_track(void *, int);
void read_replay_section(void *, int, int *, Byte *);
void construct_light_glow(void *);
void destroy_light_glow(void *);
void bind_light_glows(void *, Dword *, Dword *);
void unload_light_glows(void *);
void configure_vehicle_detail(void *);
void *construct_actor_list(void *);
void clear_actor_list(void *);
void *actor_at(void *, int);
void *list_node_at(void *, int);
void unlink_list_node(void *, void *);
void release_actor_list_base();
void create_character_table(void *);
void reset_actor_render_state(void *);
void *construct_gob_definitions(void *);
void destroy_gob_definitions(void *);
void *construct_gob_database(void *);
void destroy_gob_database(void *);
void load_gob_database(void *);
int count_path_locations(void *);
void *construct_path_location(void *);
void destroy_path_location(void *);
void *construct_path_data(void *);
void destroy_path_data(void *);
void load_path_data(void *, const void *);
void interpolate_path(int, const float *, int, float *);
void build_path_tangents(const float *, float *, int);
void fix_level_points(void *);
void *construct_boom(void *);
void destroy_boom(void *);
void finish_track_loading();
using CharacterSequenceCallback = void(__cdecl *)(void *);
void __cdecl character_frame_callback(void *);
Dword *relocate_level_gob_blocks(void *, Dword *);
void *create_car(void *, const char *, Dword, Dword, Dword, Dword, Dword);
namespace pending {
// World subsystem interfaces.
extern const Dword actor_list_methods[], character_table_methods[],
    actor_list_base_methods[], start_line_methods[],
    camera_modes_methods[], camera_director_methods[];
extern const Dword boom_base_methods[], boom_methods[];
extern Dword *gob_definitions;
extern Dword track_load_begin, track_load_end;
void initialize_animation_factory_entry(Dword, Dword);
void shutdown_animation_factory_entry();
void reset_animation_scratch_entry();
void load_character_assets(void *, int);
void release_vehicle_character_bindings_entry(void *);
void initialize_vehicle_shader_parameters();
void release_vehicle_shader_parameters();
void load_vehicle_shared_textures(void *);
void load_vehicle_catalog(void *);
void *construct_vehicle_renderable(void *);
void destroy_vehicle_renderable(void *);
void unload_vehicle_renderable(void *);
void reset_vehicle_renderable(void *);
void initialize_frontend_camera(void *);
void bind_frontend_camera(void *);
void set_camera_mode(void *, int);
void reset_network_camera(void *);
void assign_start_grid(void *);
void refresh_car_routes(void *);
void reset_car_start_position(void *);
void *construct_car(void *);
void destroy_car(void *);
void register_car_particles(void *);
void load_car_assets(void *, const char *, Dword, Dword);
Dword bind_vehicle_slot_entry(void *, Dword, Dword);
void finish_car_initialization(void *);
void load_ai_path_groups(void *);
void initialize_track_objects_entry();
void release_track_objects_entry();
Byte initialize_track_collision_grid();
void release_track_sky_entry();
void read_track_layout(void *);
void load_compartment_directory(void *);
void load_compartment(void *);
void release_compartment_entry(void *);
void unload_compartment_directory_entry(void *);
void initialize_track_positions(void *);
void bind_track_texture_state_entry(void *);
Dword legacy_clock_entry();
void resolve_text_token(const char *, Dword *);
void build_start_line_geometry(Dword *);
void register_level_texture_symbols(void *);
void unregister_level_texture_symbols(void *);
void initialize_boom_entry(void *);
Dword count_racers_ahead_for_slot(void *, int);
} // namespace pending
} // namespace nfs::game::world
