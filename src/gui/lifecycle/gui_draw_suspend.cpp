#include "gui_images.hpp"
#include "nfs/input/window/window_input.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
#include "nfs/graphics/display/graphics_error.hpp"
namespace nfs::gui::engine::images {
// This is the saved fullscreen focus/message gate.
Byte suspend_drawing() {
    if (!graphics::startup::fullscreen(&graphics::startup::settings))
        return 0;
    if (window_input::hook_snapshot_valid)
        return 1;
    MSG message;
    while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    if (!window_input::hook_get_values || !lifecycle::windows_hook)
        return 0;
    int active, unused, key, type;
    using Values = void(__cdecl *)(int *, int *, int *, int *);
    reinterpret_cast<Values>(window_input::hook_get_values)(&active, &unused, &key, &type);
    if (!active)
        return 0;
    const bool interrupted = type == WM_SYSKEYDOWN ? key == VK_TAB
                                                   : (type == WM_KEYDOWN && key == VK_ESCAPE) ||
                                                         key == VK_LWIN || key == VK_RWIN;
    if (!interrupted)
        return 0;
    reinterpret_cast<Byte &>(window_input::hook_snapshot_valid) = 1;
    graphics::suspend_rendering();
    for (;;) {
        graphics::suspend_rendering();
        if (!PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE))
            continue;
        TranslateMessage(&message);
        DispatchMessageA(&message);
        switch (message.message) {
        case WM_DESTROY:
        case WM_QUIT:
            return 1;
        case WM_ACTIVATE:
            if (short(message.wParam) != 0)
                return 1;
            break;
        case WM_ACTIVATEAPP:
            if (message.wParam != 0)
                return 1;
            break;
        }
    }
}
} // namespace nfs::gui::engine::images
