#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::frame_clock {
extern kernel::Dword ticks;
extern std::uint8_t paused;
extern float elapsed,step;
void __cdecl tick();
void initialize();
}
