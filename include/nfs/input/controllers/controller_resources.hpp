#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::controllers {
using kernel::Dword;
using Byte = std::uint8_t;
extern void *primary, *secondary, *profiles, *bindings, *devices;

void clear_devices(void *);
void clear_profiles(void *);
Dword *get_bindings();
void stop_device_effects(void *);
namespace pending {
void set_rumble(Dword, std::uint16_t, std::uint16_t);
}
} // namespace nfs::controllers
