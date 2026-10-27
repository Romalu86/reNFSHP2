#pragma once
#include "nfs/world/lifecycle/world_lifecycle.hpp"
namespace nfs::game::world::track_render {
extern float glow_visibility[2];
void draw_light_glow(void *, void *, const float *, const float *, const float *);
void draw_compartment_glows(void *, const float *);
void compose_track_transform(float *, float, float, float, float, float, float, float, float, float);
void clear_layer_flags(void *);
void draw_opaque_layers(void *, Byte);
void draw_surface_layers(void *, Byte);
void draw_reflection_layers(void *);
void read_layer_sphere(void *, float *, float *);
void draw_layer_glows(void *);
void prepare_track_directory(void *);
void prepare_reflection_directory(void *);
void draw_track_directory(void *, void *);
void draw_reflection_directory(void *, void *);
void draw_directory_overlay(void *, void *);
void draw_start_nodes(void *, const float *);
void draw_start_line(void *, void *);
void draw_opaque_track(void *, void *);
void draw_reflection_track(void *, void *);
void draw_track_glows(void *, void *);
void draw_track_overlay(void *, void *);
void apply_track_fog(void *, Dword, void *, Dword *);
}
