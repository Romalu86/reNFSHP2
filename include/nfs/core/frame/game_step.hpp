#pragma once
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::game {
using kernel::Dword;
extern Dword stop_requested, race_ticks;
extern std::uint8_t count_race_ticks;
extern float simulation_time;
extern void *tick_services[2];
extern Dword keyboard_snapshot;
extern std::uint8_t phase_reset, phase_pending, phase_locked;
extern Dword start_deadline, countdown_ticks;
void advance();
void poll_input();
Dword set_device_snapshot(void *, Dword, Dword);
namespace pending {
void step_systems(Dword);
}
} // namespace nfs::game
