#pragma once
#include "../flow/frontend_application_widgets.hpp"
#include "../flow/frontend_runtime.hpp"
#include "../flow/frontend_toggle_widget.hpp"
#include "../../gui/widgets/gui_widget_edit.hpp"
#include "../../gui/widgets/gui_widget_settings.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
namespace nfs::gui::engine::pending {
Rectangle *intersect_rectangle(Rectangle *, const Rectangle *, const Rectangle *);
}
namespace nfs::application::frontend {
using gui::widgets::family::Button;
using gui::widgets::family::ListItems;
struct TypeSpinControl {
    GeneralWidget base;
    gui::String list_name;
    Widget *display;
    Dword previous_button, next_button, reserved_128, binding;
    ListItems items;
};
struct OptionsView {
    GeneralWidget base;
    int selected_tab;
    Widget *content;
};
struct ChallengeView {
    GeneralWidget base;
    int selected;
    Widget *content;
};
struct MenuSpinButton {
    Button base;
    int highlight_duration, highlight_ticks;
};
struct RolloverButton {
    Button base;
    Dword latched;
};
struct TextWheel {
    GeneralWidget base;
    Dword text_style, password_character;
    int margin, line_spacing;
    int character_pitch, scroll_offset, selected_character;
};
struct TypeWriter {
    gui::widgets::family::EditText base;
    TextWheel *wheel;
    Dword previous_button, next_button;
};
static_assert(sizeof(TypeSpinControl) == 348 && offsetof(TypeSpinControl, items) == 304);
static_assert(sizeof(OptionsView) == 272 && sizeof(ChallengeView) == 272);
static_assert(sizeof(MenuSpinButton) == 320 && sizeof(RolloverButton) == 316);
static_assert(sizeof(TextWheel) == 292 && sizeof(TypeWriter) == 340);
extern const std::array<Method, 47> type_spin_methods, options_view_methods, challenge_view_methods;
extern const std::array<Method, 52> menu_spin_methods, rollover_methods;
extern const std::array<Method, 55> typewriter_methods;
extern const std::array<Method, 48> text_wheel_methods;
extern const std::array<Method, 55> profile_edit_methods;
const char *__cdecl type_spin_class_key();
const char *__cdecl options_view_class_key();
const char *__cdecl menu_spin_class_key();
const char *__cdecl challenge_view_class_key();
const char *__cdecl rollover_class_key();
const char *__cdecl text_wheel_class_key();
void *create_profile_edit(void *);
void __fastcall draw_type_spin(TypeSpinControl *, void *, void *);
void __fastcall initialize_type_spin(TypeSpinControl *, void *);
Dword __fastcall type_spin_event(TypeSpinControl *, void *, Dword, Dword, Dword);
void __fastcall type_spin_child_event(TypeSpinControl *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall load_type_spin(TypeSpinControl *, void *, void *);
Dword __fastcall save_type_spin(TypeSpinControl *, void *, void *);
void __fastcall initialize_options_view(OptionsView *, void *);
void __fastcall update_options_view(OptionsView *, void *, Dword);
void __fastcall initialize_challenge_view(ChallengeView *, void *);
void __fastcall update_challenge_view(ChallengeView *, void *, Dword);
void __fastcall release_challenge_view(ChallengeView *, void *);
Dword __fastcall challenge_view_event(ChallengeView *, void *, Dword, Dword, Dword);
Dword __fastcall menu_spin_state(MenuSpinButton *, void *);
void __fastcall update_menu_spin(MenuSpinButton *, void *, Dword);
Dword __fastcall load_menu_spin(MenuSpinButton *, void *, void *);
Dword __fastcall save_menu_spin(MenuSpinButton *, void *, void *);
void __fastcall initialize_typewriter(TypeWriter *, void *);
Dword __fastcall typewriter_event(TypeWriter *, void *, Dword, Dword, Dword);
void __fastcall draw_typewriter(TypeWriter *, void *, gui::engine::Engine *);
void __fastcall draw_text_wheel(TextWheel *, void *, gui::engine::Engine *);
void __fastcall update_text_wheel(TextWheel *, void *, Dword);
Dword __fastcall load_text_wheel(TextWheel *, void *, void *);
Dword __fastcall save_text_wheel(TextWheel *, void *, void *);
void rotate_text_wheel(TextWheel *, int);
void latch_menu_spin(Dword);
struct OptionPage {
    const char *name, *type;
    Dword title;
};
struct ChallengePage {
    const char *name, *type;
    Dword title, players, cops, opponents, opponent_class, skill, traffic, load_tournament, races,
        time_limit, reserved;
};
static_assert(sizeof(OptionPage) == 12 && sizeof(ChallengePage) == 52);
extern const OptionPage option_pages[];
extern ChallengePage challenge_pages[];
extern Dword last_challenge_selection;
extern const gui::engine::pending::FlagDefinition type_spin_bindings[];
bool read_catalog_value(void *, const gui::String *, Dword *);
Widget *find_menu_ancestor(Widget *, const gui::String *, const gui::String *, Dword);
const char *__cdecl options_screen_class_key();
const char *__cdecl challenge_screen_class_key();
void set_options_heading(Widget *, Dword);
void set_challenge_heading(Widget *, Dword);
Dword __cdecl update_options_heading(void *, Dword, Dword, Dword);
Dword __cdecl update_challenge_heading(void *, Dword, Dword, Dword);
Dword selected_toggle_value(ProfileToggle *);
Dword select_toggle_value(ProfileToggle *, Dword);
} // namespace nfs::application::frontend
