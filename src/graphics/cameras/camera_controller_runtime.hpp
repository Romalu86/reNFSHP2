#pragma once
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/timing/timer.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world::camera {
extern const Dword controller_methods[], position_base_methods[], network_position_methods[];
extern const float *up_direction;
extern Dword selected_modes[2];
void *construct_controller(void *);
void destroy_controller(void *);
void __fastcall advance_network_position(void *, void *, void *);
void push_mode(void *, Dword);
void bind_mode_one(void *, void *);
void bind_mode_two(void *, void *);
void bind_mode_zero(void *, void *);
void bind_follow_car(void *, void *);
void bind_mode_three_body(void *, void *);
void bind_mode_three_position(void *, void *);
void set_mode_three_offset(void *, Dword, Dword, float);
} // namespace nfs::game::world::camera
namespace nfs::game::world::camera_layout {
void refresh_mode_one(void *);
void refresh_mode_zero(void *);
void refresh_mode_two(void *);
void refresh_mode_three(void *);
} // namespace nfs::game::world::camera_layout
