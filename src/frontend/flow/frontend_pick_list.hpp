#pragma once
#include "frontend_runtime.hpp"
#include "frontend_toggle_widget.hpp"
#include "../../gui/widgets/gui_widget_combo.hpp"
#include "../../gui/widgets/gui_widget_hooks.hpp"
#include "../../gui/lifecycle/gui_configuration_values.hpp"
namespace nfs::gui::engine::pending {
int string_integer(const String *);
}
namespace nfs::gui::widgets {
Dword bind_string_reference(String *, Dword);
}
namespace nfs::application::frontend {
struct PickList {
    gui::widgets::family::EditText edit;
    Dword button, fitted_rows, combo_style;
    int selected_row;
    gui::String label;
    Dword default_value, binding;
};
struct LapList {
    PickList base;
    gui::String custom_label;
};
static_assert(sizeof(PickList) == 372 && offsetof(PickList, label) == 344 &&
              offsetof(PickList, selected_row) == 340);
static_assert(sizeof(LapList) == 392 && offsetof(LapList, custom_label) == 372);
extern const std::array<Method, 57> pick_list_methods, lap_list_methods;
extern const gui::engine::pending::FlagDefinition pick_list_bindings[34];
const char *__cdecl pick_list_class_key();
const char *__cdecl lap_list_class_key();
PickList *construct_pick_list(PickList *, void *);
void destroy_pick_list(PickList *);
void __fastcall initialize_pick_list(PickList *, void *);
Dword __fastcall pick_list_event(PickList *, void *, Dword, Dword, Dword);
void __fastcall commit_pick_list(PickList *, void *);
Dword __fastcall load_pick_list(PickList *, void *, void *);
Dword __fastcall save_pick_list(PickList *, void *, void *);
Dword select_pick_text(PickList *, const gui::String *);
Dword select_pick_row(PickList *, int);
Dword select_pick_value(PickList *, Dword);
Dword add_pick_value(PickList *, Dword);
void erase_pick_value(PickList *, Dword);
Dword enable_pick_value(PickList *, Dword, Dword);
void filter_pick_values(PickList *, const ToggleFilter *);
void __fastcall fit_pick_list(PickList *, void *, Dword);
LapList *construct_lap_list(LapList *, void *);
void destroy_lap_list(LapList *);
void __fastcall initialize_lap_list(LapList *, void *);
Dword __fastcall lap_list_event(LapList *, void *, Dword, Dword, Dword);
void __fastcall commit_lap_list(LapList *, void *);
Dword select_lap_value(LapList *, int);
void add_custom_lap(LapList *, int);
void remove_custom_laps(LapList *);
void filter_lap_values(LapList *, const ToggleFilter *);
gui::widgets::Widget *probe_widget_type(gui::widgets::TypeName,
                                        gui::widgets::Widget *);
inline gui::widgets::Widget *pick_widget(PickList *value) {
    return &value->edit.base.base;
}
inline gui::widgets::Widget *lap_widget(LapList *value) {
    return pick_widget(&value->base);
}
inline Dword selected_pick_value(PickList *value) {
    const auto *item = gui::widgets::family::combo_item(pick_widget(value), value->selected_row);
    return item ? item->data : Dword(-1);
}
struct PickListConfiguration {
    alignas(4) std::byte storage[56];
    explicit PickListConfiguration(const gui::String *section) {
        {
            pending::LocalFrontendString filename("lists.ini");
            gui::engine::pending::create_configuration(storage, &filename.value);
        }
        try {
            gui::engine::pending::select_section(storage, gui::string_ascii(section));
        } catch (...) {
            gui::engine::pending::destroy_configuration(storage);
            throw;
        }
    }
    ~PickListConfiguration() {
        gui::engine::pending::destroy_configuration(storage);
    }
};
// Shared spelling of the identical filtering loops; preserves catalogue order and appends once
// for every matching entry, including duplicate filter values.
void populate_filtered_pick_list(PickList *, const ToggleFilter *);
} // namespace nfs::application::frontend
