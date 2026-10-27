#pragma once
#include "../widgets/gui_widgets.hpp"
namespace nfs::gui {
extern Dword registry_time;
void register_input_object(void *);
} // namespace nfs::gui
namespace nfs::gui::input {
using widgets::Widget;
using Callback = Dword(__cdecl *)(Widget *, Dword, Dword, Dword);
struct Device {
    const engine::Method *methods;
    Dword type, repeat_time, repeat_count;
};
struct Keyboard {
    Device base;
    char character;
    std::uint8_t padding;
    std::uint16_t wide_character;
    Callback callback;
    Dword down[256], held[256];
    Dword modifiers, index;
};
struct MouseState {
    int x, y, wheel;
    Dword buttons[4];
};
struct Mouse {
    Device base;
    Dword released_at[4];
    MouseState current, previous;
    Dword wheel_enabled;
};
static_assert(sizeof(Device) == 16 && sizeof(Keyboard) == 2080 && sizeof(Mouse) == 92);
static_assert(offsetof(Keyboard, modifiers) == 2072 && offsetof(Mouse, current) == 32);
extern Keyboard *active_keyboard;
extern engine::Point pointer_coordinates;
extern Dword pointer_moved_at, mouse_modifiers, repeat_interval, initial_repeat_delay,
    double_click_ticks;
extern Dword debugger_present;
extern const engine::Method keyboard_methods[4], mouse_methods[5], device_methods[3];
Keyboard *construct_keyboard(Keyboard *, Dword);
Mouse *construct_mouse(Mouse *);
void destroy_keyboard(Keyboard *);
void destroy_mouse(Mouse *);
void set_character(Keyboard *, char);
void set_key(Keyboard *, Dword, Dword);
void set_mouse_button(Mouse *, Dword, Dword);
void set_mouse_axes(Mouse *, int, int, int);
Dword modifiers();
Dword pointer_target(Widget *);
Dword __fastcall poll_keyboard(Keyboard *, void *, Widget *);
Widget *__fastcall poll_mouse(Mouse *, void *, Widget *);
void __fastcall reset_keyboard(Keyboard *, void *);
void __fastcall reset_mouse(Mouse *, void *);
void __fastcall platform_reset_keyboard(Keyboard *, void *);
Dword __fastcall platform_read_keyboard(Keyboard *, void *);
Dword __fastcall platform_read_mouse(Mouse *, void *);
Dword __fastcall platform_reset_wheel(Mouse *, void *);
} // namespace nfs::gui::input
