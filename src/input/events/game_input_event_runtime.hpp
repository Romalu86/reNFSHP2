#pragma once
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/physics/contacts/contact_dependencies.hpp"
namespace nfs::services::input_events {
using Byte = std::uint8_t;
using kernel::Dword;
void store_steering_input(float *, int);
void store_reset_speed(float *, float);
int cycle_camera(void *);
Byte reset_allowed(void *);
Dword request_previous_gear(void *);
Dword request_gear(void *, int);
} // namespace nfs::services::input_events
