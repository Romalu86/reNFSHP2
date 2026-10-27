#include "nfs/input/window/window_input.hpp"
#include "nfs/core/memory/memory_primitives.hpp"

namespace nfs::window_input::handlers {
// Window callbacks use the six-argument stdcall signature.
 


Dword __stdcall create(Dword, HWND, Dword, WPARAM, LPARAM, Dword) {
    if (window_state.hook_flags) {
        if (!keyboard_hook)
            keyboard_hook = SetWindowsHookExA(2, keyboard_filter, GetModuleHandleA(nullptr), 0);
        if (!low_level_hook)
            low_level_hook = SetWindowsHookExA(13, low_level_filter, GetModuleHandleA(nullptr), 0);
    }
    return 0;
}

Dword __stdcall activate_application(Dword, HWND, Dword, WPARAM active, LPARAM, Dword) {
    application_active = active == 1;
    return 0;
}

Dword __stdcall destroy(Dword, HWND, Dword, WPARAM, LPARAM, Dword) {
    if (keyboard_hook)
        UnhookWindowsHookEx(keyboard_hook);
    if (low_level_hook)
        UnhookWindowsHookEx(low_level_hook);
    keyboard_hook = low_level_hook = nullptr;
    return 0;
}

Dword __stdcall activate_window(Dword context, HWND handle, Dword message, WPARAM state, LPARAM lparam,
                               Dword output) {
    if (context && window == handle) {
        auto *result = reinterpret_cast<LRESULT *>(output);
        if (std::uint16_t(state) == 0 || state >> 16) {
            if (window_active)
                window_active = 0;
            if (system_key) {
                if (keyboard_hook) {
                    UnhookWindowsHookEx(keyboard_hook);
                    keyboard_hook = nullptr;
                }
                if (low_level_hook) {
                    UnhookWindowsHookEx(low_level_hook);
                    low_level_hook = nullptr;
                }
            }
            ShowCursor(1);
            kernel::memory_fill(key_states, 0, 128);
            *result = DefWindowProcA(handle, message, state, lparam);
        } else {
            *result = DefWindowProcA(handle, message, state, lparam);
            window_active = 1;
            if (system_key) {
                if (!keyboard_hook)
                    keyboard_hook =
                        SetWindowsHookExA(2, keyboard_filter, GetModuleHandleA(nullptr), 0);
                if (!low_level_hook)
                    low_level_hook =
                        SetWindowsHookExA(13, low_level_filter, GetModuleHandleA(nullptr), 0);
            }
        }
    }
    return 1;
}

Dword __stdcall close(Dword context, HWND handle, Dword, WPARAM, LPARAM, Dword) {
    if (context && window == handle)
        PostQuitMessage(0);
    return 1;
}

Dword __stdcall request_destroy(Dword context, HWND handle, Dword, WPARAM, LPARAM, Dword) {
    if (context && window == handle)
        destroy_requested = 1;
    return 1;
}

Dword __stdcall system_command(Dword context, HWND, Dword, WPARAM command, LPARAM, Dword) {
    if (command == 61536 || command == 61472 || command == 61488 || command == 61728)
        return 0;
    return reinterpret_cast<WindowState *>(context)->hook_flags;
}

Dword __stdcall settings_change(Dword context, HWND handle, Dword, WPARAM, LPARAM, Dword output) {
    if (context && window == handle && reinterpret_cast<WindowState *>(context)->hook_flags)
        set_system_shortcuts(0);
    *reinterpret_cast<LRESULT *>(output) = 0;
    return 0;
}

Dword __stdcall moved(Dword context, HWND handle, Dword, WPARAM, LPARAM, Dword) {
    auto *state = reinterpret_cast<WindowState *>(context);
    if (context && window == handle && !state->hook_flags) {
        RECT bounds;
        GetClientRect(state->handle, &bounds);
        ClientToScreen(state->handle, reinterpret_cast<POINT *>(&bounds));
        state->origin_x = bounds.left;
        state->origin_y = bounds.top;
        if (redraw)
            redraw();
    }
    return 0;
}

Dword __stdcall character(Dword context, HWND handle, Dword, WPARAM value, LPARAM flags, Dword) {
    if (context && window == handle) {
        Dword count = Dword(flags) & 65535;
        if (key_capacity < static_cast<int>(count))
            count = key_capacity;
        while (count--)
            enqueue_key(std::uint16_t(value), Byte(Dword(flags) >> 16 & 127));
    }
    return 1;
}

Dword __stdcall key_down(Dword context, HWND handle, Dword message, WPARAM value, LPARAM flags, Dword) {
    if (context && window == handle) {
        Dword count = Dword(flags) & 65535;
        if (key_capacity < static_cast<int>(count))
            count = key_capacity;
        Dword scan = Dword(flags) >> 16 & 127;
        if (scan_remap)
            scan = scan_remap[scan];
        key_states[scan] = 1;
        Dword character = 0;
        if (!key_events_enabled) {
            character = navigation_key(message, value);
            if (!character)
                return 0;
        }
        while (count--)
            enqueue_key(std::uint16_t(character), Byte(scan));
    }
    return 1;
}

Dword __stdcall key_up(Dword context, HWND handle, Dword, WPARAM, LPARAM flags, Dword) {
    if (context && window == handle) {
        Dword scan = Dword(flags) >> 16 & 127;
        if (scan_remap)
            scan = scan_remap[scan];
        if (scan != 42 && scan != 54)
            key_states[scan] = 0;
        else
            key_states[42] = key_states[54] = 0;
    }
    return 0;
}
namespace {
Dword mouse_state(WPARAM value) {
    return Dword((value & 1) != 0) | (value & 2 ? 2u : 0u) | (value & 16 ? 4u : 0u);
}
} // namespace

Dword __stdcall mouse_down(Dword context, HWND handle, Dword, WPARAM buttons, LPARAM position, Dword) {
    if (context && window == handle)
        update_cursor(Dword(position) & 65535, Dword(position) >> 16, mouse_state(buttons));
    return 0;
}

Dword __stdcall mouse_move(Dword context, HWND handle, Dword, WPARAM buttons, LPARAM position, Dword) {
    if (context && window == handle)
        update_cursor(Dword(position) & 65535, Dword(position) >> 16, mouse_state(buttons));
    return 0;
}

Dword __stdcall paint(Dword context, HWND handle, Dword, WPARAM, LPARAM, Dword output) {
    if (context && window == handle && handle) {
        auto *state = reinterpret_cast<WindowState *>(context);
        if (state->hook_flags)
            SetForegroundWindow(handle);
        kernel::thread_gate(1);
        PAINTSTRUCT paint;
        BeginPaint(state->handle, &paint);
        EndPaint(state->handle, &paint);
        kernel::thread_gate(0);
        if (redraw && !state->hook_flags)
            redraw();
        *reinterpret_cast<LRESULT *>(output) = 1;
    }
    return 1;
}

Dword __stdcall erase_background(Dword context, HWND handle, Dword, WPARAM, LPARAM, Dword) {
    auto *state = reinterpret_cast<WindowState *>(context);
    return context && window == handle && handle == state->handle && state->hook_flags ? 1 : 0;
}

Dword __stdcall power(Dword, HWND, Dword, WPARAM, LPARAM, Dword output) {
    *reinterpret_cast<LRESULT *>(output) = 1112363332;
    return 1;
}
} // namespace nfs::window_input::handlers
