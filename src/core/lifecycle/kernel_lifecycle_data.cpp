#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::lifecycle {
Manager *manager = nullptr;
int startup_error = 0;
// dedicated aliases the launch option.
Dword track_pool = 0, level_pool = 0, cars_pool = 0, audio_pool = 0, debug_pool = 0,
     character_pool = 0, replay_pool = 0, ui_pool = 0, screenshot_pool = 0, save_pool = 0,
     current_pool = 0;
HHOOK windows_hook = nullptr;
HMODULE windows_hook_module = nullptr;
Dword windows_hook_error = 0;
Dword windows_hook_install_error = 0;
} // namespace nfs::lifecycle
