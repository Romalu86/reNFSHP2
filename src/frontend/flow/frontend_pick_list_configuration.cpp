#include "frontend_pick_list.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::application::frontend {

Dword __fastcall load_pick_list(PickList *list, void *, void *configuration) {
    gui::engine::pending::read_reference_string(
        configuration, "PC_PickList.mLabel", &list->label,
        &lifecycle::construction::unresolved::shared_string);
    gui::engine::pending::read_int(configuration, "PC_PickList.mDefault",
                                   reinterpret_cast<int *>(&list->default_value), 0);
    gui::engine::pending::read_enum(configuration, "PC_PickList.mDataEnum", &list->binding,
                                    pick_list_bindings, 0);
    return gui::widgets::family::combo_box_load(pick_widget(list), nullptr,
                                                reinterpret_cast<Dword>(configuration));
}

Dword __fastcall save_pick_list(PickList *list, void *, void *configuration) {
    gui::engine::pending::write_reference_string(
        configuration, "PC_PickList.mLabel", &list->label,
        &lifecycle::construction::unresolved::shared_string);
    gui::engine::pending::write_int(configuration, "PC_PickList.mDefault", int(list->default_value),
                                    0);
    gui::engine::pending::write_enum(configuration, "PC_PickList.mDataEnum", list->binding,
                                     pick_list_bindings, 0);
    return gui::widgets::family::combo_box_save(pick_widget(list), nullptr,
                                                reinterpret_cast<Dword>(configuration));
}
} // namespace nfs::application::frontend
