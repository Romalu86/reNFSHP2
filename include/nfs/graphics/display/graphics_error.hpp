#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::graphics {
void suspend_rendering();
kernel::Dword refresh_audio_focus(kernel::Dword, kernel::Dword);
namespace pending {
extern kernel::Dword &audio_focus_disabled;
} // namespace pending
} // namespace nfs::graphics
