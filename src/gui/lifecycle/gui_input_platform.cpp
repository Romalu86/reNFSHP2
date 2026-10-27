#include "gui_input.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/resources/localization/region.hpp"
#include "nfs/input/window/window_input.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstring>
namespace nfs::gui::input {
using game::runtime::invoke;
extern const engine::Method platform_keyboard_methods[4], platform_mouse_methods[5];

static void clear_character_queue() {
    if (window_input::window_instance)
        while (window_input::read_key_queue()) {
        }
}

void __fastcall platform_reset_keyboard(Keyboard *keyboard, void *) {
    reset_keyboard(keyboard, nullptr);
    clear_character_queue();
}

Dword __fastcall platform_read_keyboard(Keyboard *keyboard, void *) {
    game::poll_input();
    auto *state = static_cast<const std::uint8_t *>(nfs::input::keyboard_get(6));
    if (!state)
        return 0;
    const int language = region::get();
    const bool suppress_menu = language != 0 && language != 6;
    for (Dword key = 0; key < 256; ++key)
        set_key(keyboard, key, key == 184 && suppress_menu ? 0 : state[key] != 0);
    const Dword num_lock = Dword(std::uint16_t(GetKeyState(VK_NUMLOCK)));
    static const struct {
        Dword scan, number, navigation;
    } keypad[] = {
#include "gui_keypad.inc"
    };
    for (const auto *key = keypad; key->scan; ++key)
        if (state[key->scan])
            set_key(keyboard, num_lock & 1 ? key->number : key->navigation, 1);
    const Dword key = nfs::input::key_reader();
    if (key) {
        if (std::uint8_t(key))
            set_character(keyboard, char(key));
        if (nfs::input::scan_code(key & 255) == 28)
            set_key(keyboard, 28, 1);
    }
    return 1;
}

Dword __fastcall platform_read_mouse(Mouse *mouse, void *) {
    const auto *state = static_cast<const DIMOUSESTATE *>(nfs::input::mouse_get(6));
    if (!state)
        return 0;
    set_mouse_axes(mouse, state->lX, state->lY, state->lZ);
    if (graphics::startup::fullscreen(&graphics::startup::settings)) {
        POINT point;
        if (GetCursorPos(&point))
            set_mouse_axes(mouse, point.x, point.y, state->lZ);
    }
    for (Dword button = 0; button < 4; ++button)
        set_mouse_button(mouse, button, state->rgbButtons[button]);
    return 1;
}

Dword __fastcall platform_reset_wheel(Mouse *, void *) {
    nfs::input::mouse_set(5, 127);
    return 1;
}

static void destroy_platform_keyboard(Keyboard *keyboard) {
    destroy_keyboard(keyboard);
}

static void destroy_platform_mouse(Mouse *mouse) {
    destroy_mouse(mouse);
}

static Keyboard *__fastcall delete_platform_keyboard(Keyboard *keyboard, void *, Dword flags) {
    destroy_platform_keyboard(keyboard);
    if (flags & 1)
        release(keyboard);
    return keyboard;
}

static Mouse *__fastcall delete_platform_mouse(Mouse *mouse, void *, Dword flags) {
    destroy_platform_mouse(mouse);
    if (flags & 1)
        release(mouse);
    return mouse;
}

static Keyboard *__fastcall delete_keyboard(Keyboard *keyboard, void *, Dword flags) {
    destroy_keyboard(keyboard);
    if (flags & 1)
        release(keyboard);
    return keyboard;
}

static Mouse *__fastcall delete_mouse(Mouse *mouse, void *, Dword flags) {
    destroy_mouse(mouse);
    if (flags & 1)
        release(mouse);
    return mouse;
}
#define METHOD(name) reinterpret_cast<engine::Method>(name)
const engine::Method platform_keyboard_methods[4] = {
    METHOD(delete_platform_keyboard), METHOD(poll_keyboard), METHOD(platform_reset_keyboard),
    METHOD(platform_read_keyboard)};
const engine::Method platform_mouse_methods[5] = {METHOD(delete_platform_mouse), METHOD(poll_mouse),
                                                  METHOD(reset_mouse), METHOD(platform_read_mouse),
                                                  METHOD(platform_reset_wheel)};
const engine::Method keyboard_methods[4] = {METHOD(delete_keyboard), METHOD(poll_keyboard),
                                            METHOD(reset_keyboard), METHOD(::nfs::assertions::pure_virtual_call)};
const engine::Method mouse_methods[5] = {METHOD(delete_mouse), METHOD(poll_mouse),
                                         METHOD(reset_mouse), METHOD(::nfs::assertions::pure_virtual_call),
                                         METHOD(::nfs::assertions::pure_virtual_call)};
#undef METHOD
void set_double_click_time(Dword);
void bind_text_resolver(Object *);
Object *text_resolver();
} // namespace nfs::gui::input
namespace nfs::gui::engine::pending {
void set_configuration_directory(const char *);
void set_export_directory(const char *);
} // namespace nfs::gui::engine::pending
namespace nfs::services::pending {

static void set_frontend_directories() {
    const char *directory = reinterpret_cast<const char *>(game::runtime::system(4)) + 2604;
    gui::engine::pending::set_configuration_directory(directory);
    gui::engine::pending::set_export_directory(directory);
}

void initialize_frontend_services() {
    using namespace gui::input;
    debugger_present = IsDebuggerPresent();
    set_frontend_directories();
    bind_text_resolver(text_resolver());
    if (graphics::context) {
        auto *keyboard = static_cast<Keyboard *>(gui::allocate(sizeof(Keyboard)));
        if (keyboard) {
            construct_keyboard(keyboard, 0);
            keyboard->base.methods = platform_keyboard_methods;
        }
        gui::register_input_object(keyboard);
        auto *mouse = static_cast<Mouse *>(gui::allocate(sizeof(Mouse)));
        if (mouse) {
            construct_mouse(mouse);
            mouse->base.methods = platform_mouse_methods;
        }
        gui::register_input_object(mouse);
    }
    set_double_click_time((Dword(GetDoubleClickTime()) << 7) / 1000);
}
} // namespace nfs::services::pending
