#include "nfs/core/application/application.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
namespace nfs::application::pending {

void recover_display() {
    graphics::startup::cursor_clipped = 0;
    ClipCursor(nullptr);
}
// The defined return is AL, consumed as a boolean.
bool recover_keyboard_hook() {
    BOOL result = TRUE;
    lifecycle::windows_hook_error = 0;
    if (lifecycle::windows_hook) {
        result = UnhookWindowsHookEx(lifecycle::windows_hook);
        lifecycle::windows_hook_error = GetLastError();
    }
    lifecycle::windows_hook = nullptr;
    if (lifecycle::windows_hook_module)
        FreeLibrary(lifecycle::windows_hook_module);
    lifecycle::windows_hook_module = nullptr;
    return result != FALSE;
}
} // namespace nfs::application::pending
