#pragma once
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/graphics/backend/eagl.hpp"
namespace nfs::game::world::dependency {
using world::at;
using world::Byte;
using world::offset;
using world::subsystem;
using world::Dword;
void configure_track_material(void *, Dword, Dword, Dword, Dword, Dword, Dword, Dword);
void bind_track_texture_state(void *);
void reset_track_directory(void *);
void release_compartment(void *);
void destroy_sun_glow(void *);
void unload_compartment_directory(void *);
void load_shared_vehicle_textures(void *);
void release_vehicle_character_bindings(void *);
void release_car(void *);
void bind_car_client(void *, void *);
Dword bind_vehicle_slot(void *, Dword, Dword);
void initialize_track_objects();
void release_track_objects();
void initialize_boom(void *);
void release_track_sky();
const void *static_track_shape(void *, int);
void insert_actor_node(void *, void *, void *);
void append_actor_node(void *, void *);
void destroy_actor_list_base(void *);
void destroy_actor_list(void *);
void destroy_character_table(void *);
void *delete_actor_list_base(void *, Dword);
void *delete_actor_list(void *, Dword);
void *delete_character_table(void *, Dword);
void *delete_start_line(void *, Dword);
void *delete_base_boom(void *, Dword);
void *delete_boom_array(void *, Dword);
void *delete_camera_director(void *, Dword);
void read_boom_position(void *, Dword *);
void *boom_definition(void *);
void read_boom_bounds(void *, Dword *);
void *current_camera_state(void *);
void reset_animation_scratch();
void initialize_animation_pool(Dword);
void release_animation_pool();
void release_animation_blocks();
void initialize_animation_factory(Dword);
void shutdown_animation_factory();
void construct_animation_bank(void *);
void destroy_animation_bank(void *);
void construct_animation_object(void *);
void initialize_clock_origin();
Dword legacy_clock();
struct AnimationState {
    Dword flags, reserved_4;
    Dword buckets[256], scratch[22], reserved_460;
    Byte *cursor;
    Dword reserved_468, reserved_46c;
    Byte *pool;
    Dword pool_bytes;
};
static_assert(sizeof(AnimationState) == 1144 && offsetof(AnimationState, buckets) == 8 &&
              offsetof(AnimationState, scratch) == 1032 &&
              offsetof(AnimationState, cursor) == 1124 && offsetof(AnimationState, pool) == 1136);
extern AnimationState animation_state;
extern void *animation_factory;
struct AnimationBlock {
    void *address;
    Dword bytes, reserved;
};
extern AnimationBlock animation_blocks[3];
extern std::uint64_t clock_origin;
extern Dword *static_references;
namespace pending {
void load_vehicle_renderable(void *, Dword, Dword);
void rebind_vehicle_character(void *);
void initialize_track_object(void *, int);
void load_track_object_audio();
void insert_grid_object(void *);
void construct_uncompressed_animation(void *);
void construct_animation_group(void *);
void update_camera_director(void *);
void initialize_camera_director(void *);
void unload_camera_director(void *);
void draw_boom(void *, Dword, void *);
void advance_boom(void *);
} // namespace pending
} // namespace nfs::game::world::dependency
