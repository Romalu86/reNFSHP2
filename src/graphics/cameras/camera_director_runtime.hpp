#pragma once
#include "camera_controller_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::camera {
extern int jump_variant;
void update_jump_camera(void *);
Byte can_begin_jump_camera(void *);
Dword pop_mode(void *);
void pop_matrix_mode(void *);
int selected_layout();
void *focused_car();
kernel::numeric::Extended route_lateral_position(void *);
kernel::numeric::Extended route_heading_alignment(void *);
void copy_actor_pose(void *, const void *);
void pitch_basis(const float *, float, float *);
void axis_rotation(const float *, float, float *);
Byte clip_camera_position(void *, float *, void *, float, Byte);
void raise_camera_above_ground(void *, float *, float);
Dword refresh_camera_ground(const float *, void *, void *);
Byte avoid_camera_vehicles(float *, void *, float);
Byte displace_from_vehicle(float *, const float *, const float *, const float *, float, Byte);
void add_road_shake(float *, float, const float *);
void add_impact_shake(float *, float, const float *);
Byte hold_unstable_follow_camera(void *, void *);
void reset_follow_histories(void *);
void read_master_configuration(void *);
void *construct_actor_transform(void *);
void *construct_actor_clip(void *);
void *construct_follow_fov(void *);
void *construct_follow_look(void *);
void *construct_mount_look(void *);
void *construct_point_look(void *);
void *construct_follow_position(void *);
void *construct_matrix_position(void *);
void *construct_mount_position(void *);
void *construct_pause_position(void *);
void *construct_follow_up(void *);
extern const Dword actor_transform_methods[];
extern const Dword actor_clip_methods[];
extern const Dword follow_fov_methods[];
extern const Dword follow_look_methods[];
extern const Dword mount_look_methods[];
extern const Dword point_look_methods[];
extern const Dword follow_position_methods[];
extern const Dword matrix_position_methods[];
extern const Dword mount_position_methods[];
extern const Dword pause_position_methods[];
extern const Dword follow_up_methods[];
extern const Dword matrix_controller_methods[];
void __fastcall end_position_behavior(void *, void *, void *);
void __fastcall advance_actor_transform(void *, void *, void *);
void __fastcall begin_actor_transform(void *, void *, void *);
void __fastcall advance_actor_clip(void *, void *, void *);
void __fastcall advance_follow_fov(void *, void *, void *);
void __fastcall begin_follow_fov(void *, void *, void *);
void __fastcall advance_follow_look(void *, void *, void *);
void __fastcall begin_follow_look(void *, void *, void *);
void __fastcall advance_mount_look(void *, void *, void *);
void __fastcall advance_point_look(void *, void *, void *);
void __fastcall advance_follow_position(void *, void *, void *);
void __fastcall begin_follow_position(void *, void *, void *);
void __fastcall advance_matrix_position(void *, void *, void *);
void __fastcall advance_mount_position(void *, void *, void *);
void __fastcall begin_mount_position(void *, void *, void *);
void __fastcall advance_pause_position(void *, void *, void *);
void __fastcall advance_follow_up(void *, void *, void *);
void __fastcall begin_matrix_controller(void *, void *);
void __fastcall end_matrix_controller(void *, void *);
void __fastcall advance_matrix_controller(void *, void *);
void *__fastcall delete_actor_transform(void *, void *, Dword);
void *__fastcall delete_actor_clip(void *, void *, Dword);
void *__fastcall delete_follow_fov(void *, void *, Dword);
void *__fastcall delete_follow_look(void *, void *, Dword);
void *__fastcall delete_mount_look(void *, void *, Dword);
void *__fastcall delete_point_look(void *, void *, Dword);
void *__fastcall delete_follow_position(void *, void *, Dword);
void *__fastcall delete_matrix_position(void *, void *, Dword);
void *__fastcall delete_mount_position(void *, void *, Dword);
void *__fastcall delete_pause_position(void *, void *, Dword);
void *__fastcall delete_follow_up(void *, void *, Dword);
void save_full_fov(void *);
void save_vertical_fov(void *);
void save_horizontal_fov(void *);
void save_full_pitch(void *);
void save_vertical_pitch(void *);
void save_horizontal_pitch(void *);
void save_full_up_tension(void *);
void save_vertical_up_tension(void *);
void save_horizontal_up_tension(void *);
void save_full_mount_pitch(void *);
void save_vertical_mount_pitch(void *);
void save_horizontal_mount_pitch(void *);
void save_full_mount_height(void *);
void save_vertical_mount_height(void *);
void save_horizontal_mount_height(void *);
void save_full_interval(void *);
void save_vertical_interval(void *);
void save_horizontal_interval(void *);
void save_full_follow_parameters(void *);
void save_vertical_follow_parameters(void *);
void save_horizontal_follow_parameters(void *);
void create_high_camera(void *);
void create_bird_camera(void *);
void create_low_camera(void *);
void create_bumper_camera(void *);
void create_pause_camera(void *);
void create_matrix_camera(void *);
void create_jump_camera(void *);
void create_actor_camera(void *);
} // namespace nfs::game::world::camera
