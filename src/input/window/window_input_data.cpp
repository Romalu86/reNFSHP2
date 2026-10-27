#include "nfs/input/window/window_input.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/core/application/application.hpp"
namespace nfs::window_input {
WindowState window_state{};
HWND &window = window_state.handle;
kernel::Lock message_lock{};
MessageEntry messages[128]{};
Dword message_count = 0;
Dword &hook_disabled = launch::field<Dword>(8627952);
std::uint8_t &hook_enabled = application::windows_2000;
Dword hook_snapshot_valid{};
HOOKPROC hook_procedure = nullptr;
FARPROC hook_get_values = nullptr;
Dword cursor_buttons = 0;
int cursor_x = 0, cursor_y = 0;
kernel::Lock keyboard_lock{};
Dword key_queue[96]{};
int key_read = 0, key_write = 0, key_capacity = 0;
std::uint8_t dispatch_enabled = 1, suppress_alt_keys = 1;
std::uint8_t application_active = 0, window_active = 0, window_initialized = 0,
             cursor_lock_initialized = 0;
Dword key_events_enabled = 0, destroy_requested = 0, window_class_users = 0;
int cursor_min_x = 0, cursor_min_y = 0, cursor_max_x = 0, cursor_max_y = 0;
Byte *scan_remap = nullptr;
Byte key_states[128] = {};
SystemKey system_key = nullptr;
HHOOK keyboard_hook = nullptr, low_level_hook = nullptr;
HINSTANCE window_instance = nullptr;
const char *window_class_name = nullptr;
Redraw redraw = nullptr;
} // namespace nfs::window_input
