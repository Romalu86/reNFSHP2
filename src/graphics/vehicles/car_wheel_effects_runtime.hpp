#pragma once
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/graphics/skids/skid_lifecycle.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::runtime::physics::contacts_pending {
void accumulate_rumble(void *, int, int, int);
void wheel_effect_pose(void *, int, float *, float *, float *);
kernel::numeric::Extended wheel_slip_strength(void *, int);
void update_wheel_skid(void *, int);
void append_wheel_skid(void *, int, int, float);
void emit_wheel_particles(void *, int, const float *, int, float);
void emit_sparks(void *, const float *, float, const float *);
Byte wheel_is_punctured(void *, Byte);
void queue_car_sound(void *, Dword, Dword, Dword, float, Dword);
void update_axle_surface_audio(void *, Byte, Dword, Dword);
} // namespace nfs::game::runtime::physics::contacts_pending
namespace nfs::game::runtime::skids::lifecycle {
Byte skid_finished(void *);
void release_skid_segment(void *);
void release_skid_path(void *);
void remove_skid_path(void *, void *);
void *allocate_skid_path(void *);
Byte append_skid_segment(void *, const float *, float, Dword, float);
Byte update_skid_path(void *, const float *, float, Dword, float);
void update_skid(void *, void *, const float *, float, float, float, float, Dword, float);
} // namespace nfs::game::runtime::skids::lifecycle
