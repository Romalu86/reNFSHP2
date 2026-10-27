#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "gui_input.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "nfs/core/timing/timer.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstring>
#include <cstdlib>
namespace nfs::gui::input {
Keyboard *active_keyboard{};
engine::Point pointer_coordinates{};
static std::uint8_t pointer_destructed{};
Dword pointer_moved_at{}, mouse_modifiers{};
Dword repeat_interval = 16, initial_repeat_delay = 48, double_click_ticks = 32;
Dword debugger_present{};
using game::runtime::invoke;
// Both string destructors restore the base table.
static void release_pointer() {
    if (!(pointer_destructed & 1)) {
        pointer_destructed |= 1;
        engine::destroy_point(&pointer_coordinates);
    }
}

static void initialize_pointer() {
    engine::construct_point(&pointer_coordinates);
    std::atexit(release_pointer);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_pointer, ".CRT$XCU_NFS_006906FC",
                       initialize_pointer);
} // namespace

static void __fastcall reset_device(Device *device, void *) {
    device->repeat_time = kernel::timer_count();
    device->repeat_count = 0;
}

static Device *__fastcall delete_device(Device *device, void *, Dword flags) {
    device->methods = reinterpret_cast<const engine::Method *>(gui::base_methods);
    if (flags & 1)
        release(device);
    return device;
}
const engine::Method device_methods[3] = {reinterpret_cast<engine::Method>(delete_device),
                                          reinterpret_cast<engine::Method>(::nfs::assertions::pure_virtual_call),
                                          reinterpret_cast<engine::Method>(reset_device)};

Keyboard *construct_keyboard(Keyboard *keyboard, Dword index) {
    keyboard->base.methods = device_methods;
    keyboard->base.type = 3;
    keyboard->base.repeat_time = kernel::timer_count();
    keyboard->index = index;
    keyboard->base.repeat_count = 0;
    keyboard->callback = nullptr;
    keyboard->character = 0;
    keyboard->wide_character = 0;
    keyboard->modifiers = 0;
    keyboard->base.methods = keyboard_methods;
    std::memset(keyboard->down, 0, sizeof(keyboard->down));
    std::memset(keyboard->held, 0, sizeof(keyboard->held));
    active_keyboard = keyboard;
    return keyboard;
}

void destroy_keyboard(Keyboard *keyboard) {
    active_keyboard = nullptr;
    keyboard->base.methods = reinterpret_cast<const engine::Method *>(gui::base_methods);
}

Mouse *construct_mouse(Mouse *mouse) {
    mouse->base.methods = device_methods;
    mouse->base.type = 2;
    mouse->base.repeat_time = kernel::timer_count();
    mouse->base.repeat_count = 0;
    mouse->base.methods = mouse_methods;
    mouse->wheel_enabled = 1;
    std::memset(mouse->released_at, 0, sizeof(mouse->released_at));
    std::memset(&mouse->previous, 0, sizeof(mouse->previous));
    std::memset(&mouse->current, 0, sizeof(mouse->current));
    return mouse;
}

void destroy_mouse(Mouse *mouse) {
    mouse->base.methods = reinterpret_cast<const engine::Method *>(gui::base_methods);
}

void set_character(Keyboard *keyboard, char value) {
    if (value) {
        keyboard->character = value;
        keyboard->wide_character = 0;
    }
}

void set_key(Keyboard *keyboard, Dword key, Dword down) {
    keyboard->down[key & 255] = down != 0;
}

void set_mouse_button(Mouse *mouse, Dword button, Dword down) {
    if (button < 4)
        mouse->current.buttons[button] = down != 0;
}

void set_mouse_axes(Mouse *mouse, int x, int y, int wheel) {
    mouse->current.x = x;
    mouse->current.y = y;
    mouse->current.wheel = wheel;
}

Dword modifiers() {
    return active_keyboard ? active_keyboard->modifiers | mouse_modifiers : mouse_modifiers;
}

void __fastcall reset_keyboard(Keyboard *keyboard, void *) {
    std::memset(keyboard->held, 0, sizeof(keyboard->held));
    std::memset(keyboard->down, 0, sizeof(keyboard->down));
    keyboard->character = 0;
    keyboard->wide_character = 0;
    keyboard->modifiers = 0;
    keyboard->base.repeat_time = kernel::timer_count();
    keyboard->base.repeat_count = 0;
}

void __fastcall reset_mouse(Mouse *mouse, void *) {
    std::memset(&mouse->previous, 0, sizeof(mouse->previous));
    std::memset(&mouse->current, 0, sizeof(mouse->current));
    if (invoke<Dword>(mouse, 3))
        mouse->previous = mouse->current;
    invoke(mouse, 4);
    mouse_modifiers = 0;
    pointer_moved_at = kernel::timer_count();
    std::memset(mouse->released_at, 0, sizeof(mouse->released_at));
    mouse->base.repeat_time = kernel::timer_count();
    mouse->base.repeat_count = 0;
}

void set_double_click_time(Dword ticks) {
    double_click_ticks = ticks;
}

void set_repeat_times(Dword interval, Dword delay) {
    repeat_interval = interval;
    initial_repeat_delay = delay;
}

Dword pointer_target(Widget *root) {
    if (application::pending::captured_widget_id)
        return application::pending::captured_widget_id;
    if (context_state)
        if (auto *modal = static_cast<Widget *>(events::focused(context_state)))
            return widgets::hit_branch(modal);
    return widgets::hit_branch(root);
}
} // namespace nfs::gui::input
namespace nfs::gui::widgets::pending {

Point *pointer_position() {
    return &gui::input::pointer_coordinates;
}
} // namespace nfs::gui::widgets::pending
