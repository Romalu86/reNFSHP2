#pragma once
#include "controller_configuration.hpp"
namespace nfs::controllers {
using Method = gui::engine::Method;
struct ControlValue {
    const Method *methods;
    Dword index, reference, value;
};
struct AxisValue {
    const Method *methods;
    Dword index, reference, polarity, minimum, maximum;
};
struct AxisList {
    const Method *methods;
    Dword count;
    AxisValue values[24];
};
struct ButtonList {
    const Method *methods;
    Dword count;
    ControlValue values[24];
};
struct PovList {
    const Method *methods;
    Dword count;
    ControlValue values[4];
};
struct Controller {
    const Method *methods;
    int device;
    char name[64];
    Byte active, padding[3];
};
struct Joystick {
    Controller base;
    AxisList axes;
    ButtonList buttons;
    PovList povs;
};
struct Keyboard {
    Controller base;
    void *keys;
};
struct KeyboardName {
    Dword code, reference;
    const char *name;
};
static_assert(sizeof(ControlValue) == 16 && sizeof(AxisValue) == 24 && sizeof(Controller) == 76);
static_assert(sizeof(Joystick) == 1124 && sizeof(Keyboard) == 80);
static_assert(offsetof(Joystick, buttons) == 660 && offsetof(Joystick, povs) == 1052);
extern const Method value_base_methods[11], key_value_methods[11], axis_value_methods[11],
    button_value_methods[11], pov_value_methods[11];
extern const Method list_base_methods[10], key_list_methods[10], axis_list_methods[10],
    button_list_methods[10], pov_list_methods[10];
extern const Method controller_methods[9], keyboard_methods[9], joystick_methods[9];
extern const KeyboardName keyboard_names[125];
extern const InputAction default_map_fields[48];
struct KeyboardValues {
    ControlValue values[124];
};
extern KeyboardValues keyboard_values;
ControlValue *construct_control(ControlValue *);
AxisValue *construct_axis(AxisValue *);
ControlValue *construct_button(ControlValue *);
void set_control_reference(ControlValue *, Dword);
void set_key(ControlValue *, Dword, Dword);
void set_axis(AxisValue *, Dword, Dword, Dword, Dword, Dword);
void set_pov(ControlValue *, Dword, Dword, Dword);
Controller *construct_controller(Controller *, int, const char *);
AxisList *construct_axis_list(AxisList *);
ButtonList *construct_button_list(ButtonList *);
PovList *construct_pov_list(PovList *);
void destroy_axis_list(void *);
void destroy_button_list(void *);
void destroy_pov_list(void *);
void destroy_joystick(Joystick *);
void destroy_keyboard(void *);
void destroy_key_list(void *);
Byte parse_direction(const char *);
Dword resolve_control_reference(const char *);
AxisValue *axis_slot(Joystick *, Dword);
ControlValue *button_slot(Joystick *, Dword);
ControlValue *pov_slot(Joystick *, Dword);
} // namespace nfs::controllers
