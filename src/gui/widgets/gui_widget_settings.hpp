#pragma once
#include "gui_widgets.hpp"
#include "../lifecycle/gui_configuration_values.hpp"
namespace nfs::gui::widgets {
extern const engine::pending::FlagDefinition widget_flags[], layout_flags[], general_style_flags[],
    tab_key_flags[];
namespace pending {
Dword __fastcall load_general(GeneralWidget *, void *, void *);
Dword __fastcall save_general(GeneralWidget *, void *, void *);
Dword __fastcall general_event(GeneralWidget *, void *, Dword, Dword, Dword);
void __fastcall request_general_layout(GeneralWidget *, void *);
Dword __fastcall general_key(GeneralWidget *, void *, Dword, Dword);
Dword __fastcall general_button(GeneralWidget *, void *, Dword, Dword);
Dword __fastcall enter_interface(Widget *, void *);
Dword __fastcall load_interface(Widget *, void *, void *);
Dword __fastcall save_interface(Widget *, void *, void *);
void __fastcall request_interface_layout(Widget *, void *);
Dword __fastcall interface_key(Widget *, void *, Dword, Dword);
Dword __fastcall interface_button(Widget *, void *, Dword, Dword);
void __fastcall layout_simple_text(Widget *, void *, Dword);
void __fastcall draw_simple_text(Widget *, void *, engine::Engine *);
} // namespace pending
} // namespace nfs::gui::widgets
