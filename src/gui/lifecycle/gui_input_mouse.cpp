#include "gui_input.hpp"
#include "nfs/core/timing/timer.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
namespace nfs::gui::input {
using game::runtime::invoke;

static engine::Point *to_view(engine::Point *out, const engine::Point *point) {
    engine::construct_point(out);
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    const float x = float(Extended::integer(point->x, format) *
                              Extended(engine::inverse_horizontal_scale, format) -
                          Extended(engine::horizontal_offset, format));
    const float y = float(Extended::integer(point->y, format) *
                              Extended(engine::inverse_vertical_scale, format) -
                          Extended(engine::vertical_offset, format));
    // The saved ROUND invokes FISTP with the caller's rounding control.
    out->x = int(std::lrintf(x));
    out->y = int(std::lrintf(y));
    return out;
}

Widget *__fastcall poll_mouse(Mouse *mouse, void *, Widget *root) {
    Dword now = kernel::timer_count();
    std::memset(&mouse->current, 0, sizeof(mouse->current));
    if (!invoke<Dword>(mouse, 3))
        return nullptr;
    const int x = mouse->current.x, y = mouse->current.y;
    engine::Point pixels{engine::point_methods, x, y}, view;
    to_view(&view, &pixels);
    pointer_coordinates.x = view.x;
    pointer_coordinates.y = view.y;
    engine::destroy_point(&view);
    engine::destroy_point(&pixels);
    if (!root)
        return nullptr;
    auto *target = static_cast<Widget *>(events::find_widget(context_state, pointer_target(root)));
    if (!target)
        target = root;
    mouse_modifiers = 0;
    if (mouse->current.buttons[0])
        mouse_modifiers = 4194304;
    if (mouse->current.buttons[1])
        mouse_modifiers |= 8388608;
    if (mouse->current.buttons[2])
        mouse_modifiers |= 16777216;
    if (x == mouse->previous.x && y == mouse->previous.y)
        events::post_widget(target, 9, now - pointer_moved_at, modifiers());
    else {
        const Dword dx = Dword(x) - Dword(mouse->previous.x), dy = Dword(y) - Dword(mouse->previous.y);
        events::post_widget(target, 10, (dx & 65535) | (dy << 16), modifiers());
        mouse->previous.x = x;
        mouse->previous.y = y;
        pointer_moved_at = now;
        registry_time = kernel::timer_count();
    }
    if (mouse->wheel_enabled) {
        const int wheel = mouse->current.wheel, previous_wheel = mouse->previous.wheel;
        if (wheel != previous_wheel) {
            auto *active =
                static_cast<Widget *>(events::find_widget(context_state, widgets::active_widget_id));
            if (!active)
                active = root;
            const Dword direction = Dword(wheel <= previous_wheel);
            for (int repeat = 0; repeat < 3; ++repeat)
                events::post_widget(active, 39, direction, 0);
            invoke(mouse, 4);
            mouse->current.wheel = mouse->previous.wheel = 127;
            registry_time = kernel::timer_count();
        }
    }
    for (Dword button = 0; button < 4; ++button) {
        Dword &held = mouse->previous.buttons[button];
        if (!mouse->current.buttons[button]) {
            if (held) {
                events::post_widget(target, 12, button, modifiers());
                held = 0;
                mouse->released_at[button] = kernel::timer_count();
                registry_time = kernel::timer_count();
            }
        } else if (!held) {
            Dword flags = modifiers();
            if (mouse->released_at[button] &&
                int(kernel::timer_count() - mouse->released_at[button]) < int(double_click_ticks)) {
                flags |= 33554432;
                mouse->released_at[button] = kernel::timer_count();
            }
            events::post_widget(target, 11, button, flags);
            ++held;
            mouse->base.repeat_time = kernel::timer_count();
            mouse->base.repeat_count = 0;
            registry_time = kernel::timer_count();
        } else if (kernel::timer_frequency && repeat_interval) {
            const Dword tick = kernel::timer_count();
            const Dword delay = mouse->base.repeat_count ? repeat_interval : initial_repeat_delay;
            if (mouse->base.repeat_time + delay <= tick) {
                ++mouse->base.repeat_count;
                mouse->base.repeat_time = tick;
                const Dword previous = held;
                events::post_widget(target, 11, button, (previous & 255) | modifiers());
                if (int(previous) < 255)
                    held = previous + 1;
                registry_time = kernel::timer_count();
            }
        }
    }
    if (Object *engine = active_engine())
        if (!events::dispatch(target, 1, reinterpret_cast<Dword>(engine), 0))
            invoke(engine, 50, Dword(1));
    return target;
}
} // namespace nfs::gui::input
