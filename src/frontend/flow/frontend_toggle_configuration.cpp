#include "frontend_toggle_widget.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::frontend {
#include "frontend_toggle_data.inc"

Dword __fastcall load_profile_toggle(ProfileToggle *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_string(configuration, "ToggleButton.mListName", &widget->list_name,
                &lifecycle::construction::unresolved::shared_string);
    read_int(configuration, "ToggleButton.mbPopulateFromList", &widget->populate_from_list, 1);
    read_int(configuration, "ToggleButton.mDefault",
             reinterpret_cast<int *>(&widget->default_value), 0);
    if (!read_enum(configuration, "ToggleButton.mDataEnum", &widget->binding, toggle_bindings, 0))
        read_enum(configuration, "PC_PickList.mDataEnum", &widget->binding, toggle_bindings, 0);
    return gui::widgets::family::button_load(&widget->button.base.base, nullptr,
                                             reinterpret_cast<Dword>(configuration));
}

Dword __fastcall save_profile_toggle(ProfileToggle *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_text(configuration, "ToggleButton.mListName", gui::string_ascii(&widget->list_name),
               nullptr);
    write_int(configuration, "ToggleButton.mbPopulateFromList", widget->populate_from_list, 1);
    write_int(configuration, "ToggleButton.mDefault", int(widget->default_value), 0);
    write_enum(configuration, "ToggleButton.mDataEnum", widget->binding, toggle_bindings, 0);
    return gui::widgets::family::button_save(&widget->button.base.base, nullptr,
                                             reinterpret_cast<Dword>(configuration));
}

bool toggle_string_equals(const gui::String *text, const char *ascii) {
    const void *data = gui::string_data(text);
    if (!data)
        return !ascii;
    if (!ascii)
        return false;
    return gui::string_compare_ascii(text, ascii) == 0;
}
} // namespace nfs::application::frontend
