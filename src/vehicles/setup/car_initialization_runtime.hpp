#pragma once
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/physics/contacts/contact_dependencies.hpp"
#include "../../math/precision/extended_float.hpp"
#include <initializer_list>
namespace nfs::game::world::vehicle_setup {
using kernel::numeric::Extended;
void adjust_body_vertices(Dword, void *);
void configure_body_shape(void *);
void adjust_body_for_tires(void *);
void mark_bounds_dirty(void *);
void query_render_bounds(void *, float *, float *);
void reset_engine_limit(void *);
void reset_wheel_contacts(void *);
Extended engine_torque(void *, int);
Extended brake_torque(float, Dword, int, float);
Extended tire_lateral_force(void *, void *);
Extended tire_aligning_moment(void *, void *);
Extended rolling_resistance(float);
void initialize_tire_geometry(void *);
void initialize_body_inertia(void *);
void initialize_gear_speeds(void *);
void initialize_drive_weights(void *);
void build_gear_force_curve(void *, int, float *);
Extended gear_shift_speed(void *, int);
void initialize_brake_balance(void *);
void initialize_cornering_balance(void *);
void initialize_top_speed(void *);
Dword initialize_derived_parameters(void *);
} // namespace nfs::game::world::vehicle_setup
