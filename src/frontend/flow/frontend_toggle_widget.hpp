#pragma once
#include "frontend_application_widgets.hpp"
#include "frontend_runtime.hpp"
#include "../../gui/widgets/gui_widget_controls.hpp"
#include "../../gui/lifecycle/gui_configuration_values.hpp"
namespace nfs::profiles {
const char *__cdecl profile_list_class_key();
gui::Dword select_profile_row(void *, gui::Dword);
} // namespace nfs::profiles
namespace nfs::application::frontend {
struct ProfileToggle {
    gui::widgets::family::Button button;
    gui::widgets::family::ListItems items;
    int populate_from_list;
    gui::String list_name;
    Dword default_value, binding;
};
struct ToggleFilter {
    int count;
    const Dword *values;
};
static_assert(sizeof(ProfileToggle) == 388 && offsetof(ProfileToggle, items) == 312 &&
              offsetof(ProfileToggle, binding) == 384);
extern const std::array<Method, 52> profile_toggle_methods;
extern const std::array<Method, 14> profile_toggle_items_methods;
extern const gui::engine::pending::FlagDefinition toggle_bindings[], toggle_values[];
int populate_toggle_list(ProfileToggle *);
int find_toggle_value(ProfileToggle *, Dword);
void filter_toggle_values(ProfileToggle *, const ToggleFilter *);
void replace_toggle_list(ProfileToggle *, const char *);
void __fastcall initialize_profile_toggle(ProfileToggle *, void *);
Dword __fastcall profile_toggle_event(ProfileToggle *, void *, Dword, Dword, Dword);
Dword __fastcall profile_toggle_key(ProfileToggle *, void *, Dword, Dword);
Dword __fastcall load_profile_toggle(ProfileToggle *, void *, void *);
Dword __fastcall save_profile_toggle(ProfileToggle *, void *, void *);
bool toggle_string_equals(const gui::String *, const char *);
gui::widgets::family::ListItem *construct_toggle_item(gui::widgets::family::ListItem *,
                                                      const gui::String *, Dword);
} // namespace nfs::application::frontend
