#include "gui_ime.hpp"
#include "../../movies/playback/movie_runtime.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
namespace nfs::gui::widgets::ime {
static_assert(sizeof(MSG) == 28);
HHOOK message_hook{};
Dword instance_count{}, target_id{}, codepage{};
HIMC context{};
std::uint8_t message_flag{}, hook_busy{}, unicode_composition = 1;

HWND input_window() {
    return movies::foreground_owner();
}
// Message filtering and the two WM_CHAR drains retain the order.
Dword route_window_message(HWND window, UINT message, WPARAM first, LPARAM second) {
    if (window != input_window())
        return 0;
    Dword event = 0;
    switch (message) {
    case WM_IME_SETCONTEXT:
        event = 205;
        break;
    case WM_IME_NOTIFY:
        event = 210;
        break;
    case 644:
        event = 209;
        break;
    case 645:
        event = 204;
        break;
    case 646:
        event = 201;
        break;
    case WM_IME_KEYDOWN:
        event = 212;
        break;
    case WM_INPUTLANGCHANGEREQUEST:
        event = 202;
        break;
    case WM_INPUTLANGCHANGE:
        event = 203;
        break;
    case WM_IME_STARTCOMPOSITION:
        post_target(target_id, 206, Dword(first), Dword(second));
        return 1;
    case WM_IME_ENDCOMPOSITION:
        event = 208;
        break;
    case WM_IME_COMPOSITION:
        event = 207;
        break;
    case WM_CHAR:
        if (!context || second != 1) {
            event = 211;
        } else {
            MSG queued;
            if (!IsDBCSLeadByte(BYTE(first))) {
                int remaining = 10;
                do {
                    if (!PeekMessageA(&queued, input_window(), WM_CHAR, WM_CHAR, PM_REMOVE) ||
                        --remaining == 0)
                        goto finished;
                    Sleep(0);
                } while (IsDBCSLeadByte(BYTE(queued.wParam)));
            } else {
                int remaining = 10;
                for (;;) {
                    if (!PeekMessageA(&queued, input_window(), WM_CHAR, WM_CHAR, PM_REMOVE) ||
                        remaining == 1)
                        goto finished;
                    Sleep(0);
                    if (remaining != 10)
                        break;
                    remaining = 9;
                }
            }
            post_target(target_id, 211, Dword(queued.wParam), Dword(queued.lParam));
        }
        break;
    default:
        break;
    }
    if (event)
        post_target(target_id, event, Dword(first), Dword(second));
finished:
    message_flag = 0;
    return 1;
}

static LRESULT CALLBACK message_callback(int code, WPARAM first, LPARAM second) {
    if (!hook_busy) {
        hook_busy = 1;
        if (second) {
            const MSG copy = *reinterpret_cast<const MSG *>(second);
            route_window_message(copy.hwnd, copy.message, copy.wParam, copy.lParam);
        }
        hook_busy = 0;
    }
    return CallNextHookEx(message_hook, code, first, second);
}

void install_message_hook() {
    if (!message_hook)
        message_hook =
            SetWindowsHookExA(WH_GETMESSAGE, message_callback,
                              services::pending::application_instance, GetCurrentThreadId());
}

void remove_message_hook() {
    if (message_hook) {
        UnhookWindowsHookEx(message_hook);
        message_hook = nullptr;
    }
}
} // namespace nfs::gui::widgets::ime
