#include "nfs/input/window/window_input.hpp"
#include "nfs/core/console/console.hpp"
#include <cstdlib>

namespace nfs::window_input {
static Dword saved_screensaver = 0, saved_low_power = 0;
static Dword shortcuts_saved = 0;

// Only the low return byte belongs to this contract.
std::uint8_t dispatch(HWND handle, Dword message, WPARAM wparam, LPARAM lparam, LRESULT *result) {
    bool handled = false;
    if (dispatch_enabled) {
        kernel::lock_enter(&message_lock);
        MessageEntry key{message, nullptr};
        auto compare = [](const void *left, const void *right) -> int {
            return static_cast<int>(static_cast<const MessageEntry *>(left)->message -
                                    static_cast<const MessageEntry *>(right)->message);
        };
        auto *entry = static_cast<MessageEntry *>(
            kernel::binary_search(&key, messages, message_count, 8, compare));
        if (entry)
            handled = entry->callback(reinterpret_cast<Dword>(&window_state), handle, message,
                                      wparam, lparam, reinterpret_cast<Dword>(result)) == 1;
        kernel::lock_leave(&message_lock);
    }
    return handled;
}

// The synchronous notification branch returns inverted BOOL.
bool post_message(Dword message, LPARAM lparam, WPARAM wparam, int notify) {
    if (!window)
        return false;
    if (notify)
        return SendNotifyMessageA(window, message, wparam, lparam) == 0;
    DWORD error = 0;
    if (!PostMessageA(window, message, wparam, lparam))
        error = GetLastError();
    return error == 0;
}

// Toggle using the inverse of the previous value.
void set_system_shortcuts(int enabled) {
    int screensaver, low_power;
    SystemParametersInfoA(16, 0, &screensaver, 0);
    SystemParametersInfoA(84, 0, &low_power, 0);
    if (!shortcuts_saved) {
        saved_screensaver = screensaver;
        saved_low_power = low_power;
        shortcuts_saved = 1;
    }
    if (screensaver != enabled)
        SystemParametersInfoA(17, screensaver == 0, nullptr, 2);
    if (low_power != enabled)
        SystemParametersInfoA(86, low_power == 0, nullptr, 2);
}


void restore_system_shortcuts() {
    if (shortcuts_saved) {
        SystemParametersInfoA(17, saved_screensaver, nullptr, 2);
        SystemParametersInfoA(86, saved_low_power, nullptr, 2);
    }
}

// Registration alone does not create a window or install hooks.
void initialize_messages() {
    if (window_initialized)
        return;
    kernel::lock_init(&keyboard_lock);
    key_queue_capacity(32);
    const MessageEntry defaults[] = {{1, handlers::create},
                                     {28, handlers::activate_application},
                                     {6, handlers::activate_window},
                                     {5, handlers::moved},
                                     {3, handlers::moved},
                                     {16, handlers::close},
                                     {2, handlers::destroy},
                                     {15, handlers::paint},
                                     {20, handlers::erase_background},
                                     {26, handlers::settings_change},
                                     {274, handlers::system_command},
                                     {1126, handlers::request_destroy},
                                     {258, handlers::character},
                                     {256, handlers::key_down},
                                     {260, handlers::key_down},
                                     {257, handlers::key_up},
                                     {261, handlers::key_up},
                                     {513, handlers::mouse_down},
                                     {519, handlers::mouse_down},
                                     {516, handlers::mouse_down},
                                     {515, handlers::mouse_down},
                                     {521, handlers::mouse_down},
                                     {518, handlers::mouse_down},
                                     {514, handlers::mouse_move},
                                     {520, handlers::mouse_move},
                                     {517, handlers::mouse_move},
                                     {512, handlers::mouse_move},
                                     {536, handlers::power}};
    for (const auto &entry : defaults)
        register_message(entry.message, entry.callback);
    window_initialized = 1;
}

 
void bind_window(HWND handle, HWND secondary, Dword flags) {
    if (!cursor_lock_initialized) {
        kernel::lock_init(&message_lock);
        cursor_lock_initialized = 1;
    }
    kernel::print::write(9, "setmainwindow\n");
    if (!window_initialized)
        initialize_messages();
    if (window)
        release_window();
    application_active = 1;
    window_state.hook_flags = flags;
    window_state.owned = 0;
    window = handle;
    window_state.secondary = secondary;
    window_instance = GetModuleHandleA(nullptr);
}

// The owned-window wait and task pump retain their ordering.
void release_window() {
    if (window_state.owned == 1) {
        restore_system_shortcuts();
        post_message(1126, 0, 0, 0);
        while (window) {
            kernel::thread_yield(0);
            kernel::cooperate(0);
        }
    }
    if (window_class_users && --window_class_users == 0) {
        UnregisterClassA(window_class_name, window_instance);
        cursor_lock_initialized = 0;
        kernel::lock_destroy(&message_lock);
        window_instance = nullptr;
    }
}
} // namespace nfs::window_input
