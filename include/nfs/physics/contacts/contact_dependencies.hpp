#pragma once
#include "nfs/physics/contacts/vehicle_contacts.hpp"
namespace nfs::game::runtime::physics::contact {
Dword request_next_gear(void *);
void reset_transmission(void *);
void reset_wheel_state(void *);
void reset_controls(void *);
void decode_normal(const void *, float *);
void *polygon_at(Dword *, std::uint16_t);
void polygon_corners(void *, float **);
Byte search_ground(void *, void *, const float *, void *, Byte);
Dword move_out_of_plane(void *, const float *, const float *, float *);
int apply_impulse(void *, const float *, const float *, float, Dword, Dword, float, Dword);
} // namespace nfs::game::runtime::physics::contact
