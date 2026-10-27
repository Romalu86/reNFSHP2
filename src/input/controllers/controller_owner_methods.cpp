#include "controller_interfaces.hpp"
#include "nfs/core/errors/assertions.hpp"
namespace nfs::controllers {
// Complete AL=0 leaves; no stack parameters.
static Byte __fastcall controller_axis_flag(void *, void *) {
    return 0;
}

static Byte __fastcall controller_key_flag(void *, void *) {
    return 0;
}

static Byte __fastcall controller_button_flag(void *, void *) {
    return 0;
}

static Dword __fastcall controller_key_interface(void *, void *) {
    return 0;
}

static Dword __fastcall controller_axis_interface(void *, void *) {
    return 0;
}

static Dword __fastcall controller_button_interface(void *, void *) {
    return 0;
}

static Dword __fastcall controller_pov_interface(void *, void *) {
    return 0;
}

static const char *__fastcall keyboard_controller_name(void *, void *) {
    return "keyboard";
}
// This literal is also'keyboard' in the joystick table.
static const char *__fastcall joystick_controller_name(void *, void *) {
    return "keyboard";
}

static void *__fastcall keyboard_keys(Keyboard *owner, void *) {
    return owner->keys;
}

static void *__fastcall joystick_axes(Joystick *owner, void *) {
    return &owner->axes;
}

static void *__fastcall joystick_buttons(Joystick *owner, void *) {
    return &owner->buttons;
}

static void *__fastcall joystick_povs(Joystick *owner, void *) {
    return &owner->povs;
}

static void *__fastcall delete_controller(void *owner, void *, unsigned flags) {
    at<const Method *>(owner, 0) = controller_methods;
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_keyboard(void *owner, void *, unsigned flags) {
    destroy_keyboard(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

static void *__fastcall delete_joystick(Joystick *owner, void *, unsigned flags) {
    destroy_joystick(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}
#define METHOD(value) reinterpret_cast<Method>(value)
const Method controller_methods[9] = {METHOD(delete_controller),
                                      METHOD(controller_axis_flag),
                                      METHOD(controller_key_flag),
                                      METHOD(controller_button_flag),
                                      METHOD(assertions::pure_virtual_call),
                                      METHOD(controller_key_interface),
                                      METHOD(controller_axis_interface),
                                      METHOD(controller_button_interface),
                                      METHOD(controller_pov_interface)};
const Method keyboard_methods[9] = {
    METHOD(delete_keyboard),           METHOD(controller_axis_flag),
    METHOD(controller_key_flag),       METHOD(controller_button_flag),
    METHOD(keyboard_controller_name),  METHOD(keyboard_keys),
    METHOD(controller_axis_interface), METHOD(controller_button_interface),
    METHOD(controller_pov_interface)};
const Method joystick_methods[9] = {METHOD(delete_joystick),
                                    METHOD(controller_axis_flag),
                                    METHOD(controller_key_flag),
                                    METHOD(controller_button_flag),
                                    METHOD(joystick_controller_name),
                                    METHOD(controller_key_interface),
                                    METHOD(joystick_axes),
                                    METHOD(joystick_buttons),
                                    METHOD(joystick_povs)};
#undef METHOD
} // namespace nfs::controllers
