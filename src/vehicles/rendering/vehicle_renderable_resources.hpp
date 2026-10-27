#pragma once
#include "vehicle_renderable_runtime.hpp"
#include "../../world/resources/track_render_resources.hpp"
namespace nfs::game::world::vehicle_render {
struct ResourceGroup {
    eagl::Loader *car, *shadow, *skeleton;
    void *images, *damage, *car_image, *shadow_image, *skeleton_image;
    Dword catalog, references;
};
static_assert(sizeof(ResourceGroup) == 40);
const char *catalogue_resource(void *, Dword);
Dword catalogue_index(void *, const char *);
void *catalogue_skin(void *, Dword, Dword);
Byte catalogue_addons(void *, Dword, Dword);
float catalogue_trunk_bounce(void *, Dword);
float catalogue_hood_bounce(void *, Dword);
float catalogue_wheel_speed(void *, Dword);
void load_archive_resources(void **, const char *);
void acquire_resource_group(void *, Dword, const char *, ResourceGroup *);
void release_resource_group(void *, Dword);
void *acquire_skin(void *, Dword, Dword, const char *);
void release_skin(void *, Dword, Dword);
void *load_body_model(void *, const char *, Dword, Byte);
void bind_model_states(void *, Dword *, Dword);
void set_model_layers(void *, Dword);
void set_vehicle_computation(void *, Dword);
void bind_model_fsh(void *, Dword *);
Dword *find_named_shape(Dword *, const char *);
void bind_model_shape(void *, const char *, Dword *);
void read_geometry_configuration(void *, void *, const char *);
void bind_body_configuration(void *, const char *);
void bind_body_skin(void *, Dword, Dword);
void bind_shadow_extents(void *, Dword, const float *);
void enable_body_addons(void *);
void reset_body_state(void *);
void reset_damage(void *);
void reset_driver(void *);
void set_light_channel(void *, Dword, Dword);
void unload_lights(void *);
void unload_damage(void *);
void unload_driver(void *);
void unload_attachments(void *);
void unload_shadow(void *);
void destroy_damage_zone(void *);
float damage_amount(void *, int);
float damage_limit(void *, int);
void prepare_shader_lighting(void *);
void prepare_hot_specular(void *);
void smooth_hot_specular(void *);
void render_body_passes(void *, const Dword *);
void draw_body_layer(void *, Dword);
void select_body_layers(void *);
void *body_layer(void *, int);
void enable_geometry_addons(void *);
void bind_shadow_owner(void *, Dword);
void create_attachment_node(void *, Dword);
Dword attachment_index(const char *);
void *reserve_skeleton_node(void *);
Dword *animation_pose_scratch();
Dword *animation_matrix_scratch();
Dword *animation_world_scratch();
extern const Dword skeleton_methods[1];
void *construct_skeleton(void *);
void bind_skeleton(void *, eagl::Loader *);
Byte skeleton_has_node(void *, const char *);
void insert_skeleton_node(void *, Dword, void *);
void restore_skeleton_pose(void *, Dword *, const Dword *);
void convert_skeleton_pose(void *, const float *, float *, const Dword *);
void accumulate_skeleton_matrices(void *, const float *, float *, const Dword *);
void bind_skeleton_matrices(void *, const Dword *, Dword *, const Dword *);
void apply_skeleton_bind_pose(void *, const float *, float *, const Dword *);
void evaluate_skeleton(void *, Dword *);
void *find_animation_entry(void *, Dword);
void bind_animation_channel(void *, Dword, void *, Dword, Dword);
void bind_animation_channel_rotation(void *, Dword, void *, void *, Dword, Dword);
void evaluate_animation_channel(void *, Dword *);
void sample_animation_channel(void *, Dword *, Dword);
void construct_animation_channel(void *);
void allocate_animation_channels(void *, const char *, Dword, Dword);
using SkeletonMultiply = float *(*)(float *, const float *, const float *);
extern SkeletonMultiply skeleton_multiply;
namespace pending {
float *multiply_skeleton_matrix(float *, const float *, const float *);
} // namespace pending
extern float hot_specular_amount;
extern char resource_archive_path[260];
extern const char *const attachment_names[45];
namespace pending {
void draw_vehicle_shadow(void *, Byte);
void draw_model(void *, const Dword *);
void load_attachments(void *, eagl::Loader *);
void bind_driver(void *, Dword, Byte, void *, Dword);
void evaluate_attachments(void *);
void initialize_attachment_root(void *);
void load_lights(void *, const char *, void *);
Byte has_damage_bank(const char *);
void load_damage(void *, const char *, const Dword *, const Dword *);
void select_character_animation(void *, Dword, Dword);
void release_character(void *, void *);
Dword skeleton_node_index(void *, const char *);
void *find_skeleton_node(void *, Dword);
void release_skeleton_node(void *, void *);
void initialize_driver_actor(void *);
void *construct_damage_animation_table(void *);
void load_damage_animation_table(void *, const char *, Dword, Dword);
} // namespace pending
} // namespace nfs::game::world::vehicle_render
