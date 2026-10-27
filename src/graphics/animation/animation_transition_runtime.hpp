#pragma once
#include "animation_channel_values.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
void __fastcall select_duration_transition(void *, void *, float);
void __fastcall select_frame_transition(void *, void *, float);
int __stdcall duration_cycle_index(float, float, float);
int __stdcall frame_cycle_index(float, float, float);
Extended __stdcall wrap_duration_time(float, float, float);
Extended __stdcall wrap_frame_time(float, float, float);
Extended __fastcall transition_clip_duration(void *, void *);
void blend_transition_poses(int, float, const float *, const float *, float *, const Dword *);
void blend_animation_translation(float, const float *, const float *, float *);
Dword __fastcall sample_duration_displacement(void *, void *, float, float, float *);
Dword __fastcall sample_frame_displacement(void *, void *, float, float, float *);
void __fastcall advance_duration_turn(void *, void *, int);
void __fastcall advance_frame_turn(void *, void *, int);
void __fastcall apply_duration_turn_to_pose(void *, void *, float *);
void __fastcall apply_frame_turn_to_pose(void *, void *, float *);
void __fastcall apply_duration_turn_to_displacement(void *, void *, float *);
void __fastcall apply_frame_turn_to_displacement(void *, void *, float *);
Extended duration_displacement_length(const float *);
Extended frame_displacement_length(const float *);
void duration_turn_between_directions(const float *, const float *, float *);
void frame_turn_between_directions(const float *, const float *, float *);
Dword __fastcall duration_start_direction(void *, void *, float *);
Dword __fastcall duration_end_direction(void *, void *, float *);
Dword __fastcall sample_frame_direction(void *, void *, float, float, float *);
void __fastcall sample_frame_root_rotation(void *, void *, float, float, float *);
void __fastcall clip_start_root_rotation(void *, void *, float *);
void __fastcall clip_end_root_rotation(void *, void *, float *);
void recycle_child_animation(Dword *);
dependency::AnimationBlock *animation_shared_block(Dword);
extern Dword duration_turn_updates;
} // namespace nfs::game::world::actor_resources::animation_runtime
