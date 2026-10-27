#pragma once
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/input/directinput/directinput.hpp"
namespace nfs::services {
using Byte = std::uint8_t;
void set_steering_force(void *, float, int);
void set_impact_force(void *, float, int);
void set_lateral_force(void *, float, Byte);
} // namespace nfs::services
