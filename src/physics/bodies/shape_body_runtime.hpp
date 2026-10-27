#pragma once
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::runtime::physics {
namespace pending {
void update_ground_state(void *);
}
void clear_ground_contact(void *);
void build_body_vertices(int, const float *, float *);
void set_body_material(void *, Dword);
void initialize_body_mass(void *, float, float, float, float, int, Dword);
void report_body_shape_error(Dword);
void reset_body_ground(void *, Byte);
Byte initialize_shape_body(void *, void *, Byte);
} // namespace nfs::game::runtime::physics
