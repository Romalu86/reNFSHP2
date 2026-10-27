#pragma once
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::game::runtime::physics {
void begin_track_contacts(Dword *);
void clear_car_skids(Dword *);
void reset_car_motion(Dword *);
void reset_car(Dword *);
void update_car_reset(Dword *);
void update_car_effects(Dword *);
void reset_track_vehicle(void *);
Byte sample_ground(const float *, void *);
Byte locate_ground(void *, float *, float *);
void matrix_to_quaternion(const float *, float *);
void quaternion_to_matrix(const float *, float *);
void interpolate_rotation(const float *, const float *, float, float *);
void approach_network_pose(void *);
Dword interpolate_vehicle(float, void *);
void update_car_damage(Dword *);
Dword collide_wheels(void *);
void update_track_car(void *);
void update_contacts(void *, Byte);
void predict_body_pose(float, const float *, const float *, float *, float *, const float *);
void read_track_rotation(void *, float *);
void mark_track_vehicle_moved(void *);
void submit_body_impact(void *);
namespace contacts_pending {
void find_reset_transform(void *);
void clear_car_controls(void *);
void update_collision_feedback(void *);
void update_contact_particles(void *);
void update_body_damage(void *);
void update_exhaust_effects(void *);
void update_ground_effects(void *);
Byte search_ground_cell(void *, void *, const float *, void *, Byte);
void accumulate_wheel_damage(void *, const float *, float);
void apply_surface_damage(void *, float *, float);
Dword resolve_wheel_contacts(void *, const float *, float);
int find_body_edge_contact(const float *, const float *, float *, float *, Dword *, Dword,
                           Dword);
int apply_body_contact(void *, const float *, const float *, float, Dword, Dword, Dword,
                       Dword);
extern const Dword body_contact_friction[2];
extern Dword body_contact_flags;
} // namespace contacts_pending
} // namespace nfs::game::runtime::physics
namespace nfs::game::runtime::track {
void path_direction(void *, int, float *);
long double vehicle_direction(void *);
} // namespace nfs::game::runtime::track
