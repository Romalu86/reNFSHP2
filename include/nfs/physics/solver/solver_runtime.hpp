#pragma once
#include "nfs/physics/frame/simulation_runtime.hpp"
#include <cmath>
namespace nfs::game::runtime {
namespace matrices {
void transpose(const float *, float *);
void multiply(const float *, const float *, float *);
void rotate_x(float *, float);
void rotate_y(float *, float);
void rotate_z(float *, float);
void rotate(const float *, float, float, float, float *);
void orthonormalize(float *);
const float *set_matrix(const float *);
const float *set_translation(const float *);
Dword set_output_stride(Dword);
Dword transform(Dword, int, const void *, void *);
void transform_vectors(int, const float *, const float *, float *);
void transform_positions(int, const float *, const float *, const float *, float *, Dword);
using Transform = void(__cdecl *)(int, const void *, void *);
using Postprocess = void(__cdecl *)(int, void *);
extern const float *matrix, *translation;
extern Dword matrix_changed, translation_changed, cached_flags, input_stride, output_stride;
namespace pending {
extern Transform transforms[32];
extern Postprocess postprocessors[8];
extern Dword transform_projection_matrix, transform_clip_matrix, clip_flags, clip_aux;
extern void *transform_vertex_buffer, *vertex_end;
extern void *saved_vertices, *saved_end;
extern const void *saved_input;
extern void *saved_output;
extern Dword transformed_mode;
extern Transform selected_transform_handler;
extern Postprocess postprocess;
} // namespace pending
} // namespace matrices
long double point_distance(const float *, const float *);
void add_vector4(const float *, const float *, float *);
void subtract_vector4(const float *, const float *, float *);
namespace saved {
extern const Byte atan_octant[257];
extern const float zero_vector[3], identity_matrix[9], ground_friction[2];
extern Byte surface_parameters[23 * 48];
} // namespace saved
namespace ai {
int pair_index(int, int);
float *pair_entry(Dword *, int, int);
Byte closer(void **, void *, void *);
void insertion_sort(void **, void **, void *);
void quick_partition_sort(void **, void **, void *);
void *median_vehicle(void *, void *, void *, void *);
void **partition_neighbors(void **, void **, void *, void *);
} // namespace ai
namespace physics {
namespace pending {
Byte locate_ground_entry(void *, float *, float *);
Byte sample_ground_entry(const float *, void *);
void retire_vehicle(void *);
void reset_track_vehicle_entry(void *);
Dword interpolate_vehicle_entry(Dword *, float, void *);
extern float shape_vertices[78];
void contact_force(void *, const float *, const float *, const float *, float *, float *);
float *world_shape(void *);
void *construct_ground_contact(void *);
void *surface_properties(int);
int resolve_ground_point(void *, const float *, const float *, const float *, Dword);
} // namespace pending
} // namespace physics
namespace track {
void make_compartment_path(Dword *, char *);
void relocate_collision(Dword *, Dword *);
Dword compartment_lights(Dword *);
namespace pending {
Dword load_resource(const char *, Dword, Dword);
void release_named(void *, const char *, Dword);
Dword *construct_light_glows(Dword *);
void destroy_light_glows(Dword *);
void unload_light_glows_entry(Dword *);
void bind_light_glows_entry(Dword *, void *, Dword);
} // namespace pending
} // namespace track
namespace particles {
void reset_type(Dword *);
Dword *find_texture(Dword *, const char *);
Dword *construct_pool(Dword *, Dword, Dword);
void destroy_pool(Dword *);
void enable_emitters(Dword *);
namespace pending {
void destroy_type(Dword *);
void destroy_emitter(Dword *);
void load_pool_entry(Dword *, Dword *);
Dword *construct_type(Dword *);
Dword *construct_emitter(Dword *);
Dword *allocate_type(Dword *);
Dword *allocate_emitter(Dword *);
void initialize_type(Dword *, Dword *, Dword *, Dword *);
void prepare_type(Dword *);
void initialize_emitter(Dword *, const void *);
Byte attach_type(Dword *, Dword *);
Dword *emitter_type(Dword *, int);
} // namespace pending
} // namespace particles
namespace race_audio {
int doppler(const float *, const float *, const float *);
int fixed_atan2(int, int);
Dword next_emitter_serial();
int find_emitter_sound(const char *);
void position_emitter_sound(Dword, Dword, const float *, const float *);
void stop_engine(int, int);
void start_engine(int, int, int);
void advance_engine(int, Dword, Dword, Dword, int);
Dword engine_active(int, int);
void debug_sound(int, int, const char *, ...);
namespace pending {
extern Dword *(&listener_objects)[2];
extern float (&previous_listener)[3];
extern float (&race_audio_listener_velocity)[6];
extern Byte *const engine_state;
extern Byte &engines_disabled;
extern Dword &emitter_serial;
extern int &sound_definition_count;
extern Byte *const sound_definitions;
void update_stream_emitter(const void *);
void submit_nearest(Dword, Dword, Dword, Dword, Dword, Dword);
void submit_distanced(Dword, Dword, Dword, Dword, Dword, Dword, Dword, Dword, Dword, Dword);
} // namespace pending
} // namespace race_audio
} // namespace nfs::game::runtime
