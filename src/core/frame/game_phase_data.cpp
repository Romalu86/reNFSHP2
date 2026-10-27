#include "nfs/core/frame/game_step.hpp"
namespace nfs::game {
Dword stop_requested = 0;
Dword race_ticks = 0;
std::uint8_t count_race_ticks = 0;
float simulation_time = 0;
void *tick_services[2]{};
Dword keyboard_snapshot = 0;

std::uint8_t phase_reset = 0, phase_pending = 0, phase_locked = 0;
Dword start_deadline = 0, countdown_ticks = 0;

} // namespace nfs::game
