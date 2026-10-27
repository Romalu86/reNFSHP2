#include "nfs/input/window/window_input.hpp"

namespace nfs::window_input {
namespace {
bool intercepted_key(WPARAM key, bool alt, Dword &modifier, Dword &scan) {
    if (suppress_alt_keys && alt) {
        modifier = 56;
        if (key == 9)
            scan = 15;
        else if (key == 27)
            scan = 1;
        else {
            if (GetAsyncKeyState(17) >= 0 || key != 32)
                return false;
            scan = 57;
        }
    } else if (GetAsyncKeyState(17) < 0) {
        modifier = 29;
        if (key != 27)
            return false;
        scan = 1;
    } else {
        if (key != 91 && key != 92 && key != 93 && key != 44)
            return false;
        modifier = 0;
        scan = key;
    }
    return true;
}
} // namespace
 
LRESULT CALLBACK keyboard_filter(int code, WPARAM key, LPARAM flags) {
    if (!code && application_active && system_key) {
        Dword modifier, scan;
        if (intercepted_key(key, (Dword(flags) & 536870912) != 0, modifier, scan)) {
            if (!system_key || system_key(modifier, scan))
                return 1;
        }
    }
    return CallNextHookEx(keyboard_hook, code, key, flags);
}
// Key-up events retain the original unconditional swallow.
LRESULT CALLBACK low_level_filter(int code, WPARAM message, LPARAM event) {
    if (!code && application_active && system_key) {
        const auto *key = reinterpret_cast<const KBDLLHOOKSTRUCT *>(event);
        Dword modifier, scan;
        if (intercepted_key(key->vkCode, (key->flags & 32) != 0, modifier, scan)) {
            if (!system_key || message == 261 || message == 257 || system_key(modifier, scan))
                return 1;
        }
    }
    return CallNextHookEx(low_level_hook, code, message, event);
}
} // namespace nfs::window_input
