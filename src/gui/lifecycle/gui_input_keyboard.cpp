#include "gui_input.hpp"
#include "nfs/core/timing/timer.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/application/application.hpp"
#include <cstring>
namespace nfs::gui::input {
using game::runtime::invoke;

Dword __fastcall poll_keyboard(Keyboard *keyboard, void *, Widget *root) {
    if (!root)
        return 0;
    void *modal = context_state ? events::focused(context_state) : nullptr;
    auto *target =
        static_cast<Widget *>(events::find_widget(context_state, widgets::active_widget_id));
    if (!target)
        target = root;
    std::memset(keyboard->down, 0, sizeof(keyboard->down));
    keyboard->character = 0;
    keyboard->wide_character = 0;
    if (!invoke<Dword>(keyboard, 3))
        return 0;
    keyboard->modifiers = keyboard->index << 28;
    if (keyboard->down[42])
        keyboard->modifiers |= 65536;
    if (keyboard->down[54])
        keyboard->modifiers |= 131072;
    if (keyboard->down[56])
        keyboard->modifiers |= 1048576;
    if (keyboard->down[184])
        keyboard->modifiers |= 2097152;
    if (keyboard->down[29])
        keyboard->modifiers |= 262144;
    if (keyboard->down[157])
        keyboard->modifiers |= 524288;
    const auto submit = [&](Dword event, Dword first, Dword second) {
        if (!keyboard->callback || !keyboard->callback(target, event, first, second))
            events::post_widget(target, event, first, second);
    };
    const char character = keyboard->character;
    if (character) {
        registry_time = kernel::timer_count();
        submit(5, Dword(int(character)), keyboard->modifiers);
    } else {
        const std::uint16_t wide_character = keyboard->wide_character;
        if (wide_character) {
            registry_time = kernel::timer_count();
            if (std::uint8_t(wide_character))
                submit(5, wide_character, keyboard->modifiers | 4194304);
        }
    }
    bool new_press = false, repeat = false;
    for (int key = 1; key < 256; ++key)
        if (keyboard->down[key] && !keyboard->held[key]) {
            keyboard->base.repeat_time = kernel::timer_count();
            keyboard->base.repeat_count = 0;
            new_press = true;
            break;
        }
    if (!new_press)
        for (int key = 1; key < 256; ++key)
            if (keyboard->down[key] && keyboard->held[key]) {
                if (kernel::timer_frequency && repeat_interval) {
                    const Dword now = kernel::timer_count();
                    const Dword delay =
                        keyboard->base.repeat_count ? repeat_interval : initial_repeat_delay;
                    if (keyboard->base.repeat_time + delay <= now) {
                        keyboard->base.repeat_time = now;
                        ++keyboard->base.repeat_count;
                        repeat = true;
                    }
                }
                break;
            }
    for (int key = 1; key < 256; ++key) {
        Dword &held = keyboard->held[key];
        if (!keyboard->down[key]) {
            if (held) {
                submit(8, Dword(key), keyboard->modifiers);
                held = 0;
                registry_time = kernel::timer_count();
            }
        } else if (!held) {
            const Dword flags = keyboard->modifiers;
            if (!keyboard->callback || !keyboard->callback(target, 6, Dword(key), flags)) {
                if (modal)
                    events::post_widget(modal, 7, Dword(key), flags);
                events::post_widget(target, 6, Dword(key), flags);
            }
            held = 1;
            registry_time = kernel::timer_count();
        } else if (repeat) {
            const Dword flags = keyboard->modifiers | held;
            if (!keyboard->callback || !keyboard->callback(target, 6, Dword(key), flags)) {
                if (modal)
                    events::post_widget(modal, 7, Dword(key), flags);
                events::post_widget(target, 6, Dword(key), flags);
            }
            if (int(held) < 255)
                ++held;
            registry_time = kernel::timer_count();
        }
    }
    return reinterpret_cast<Dword>(target);
}
} // namespace nfs::gui::input
