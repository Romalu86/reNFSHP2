#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_pick_list.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

PickList *construct_pick_list(PickList *list, void *parent) {
    gui::widgets::pending::construct_combo_box(list, parent);
    try {
        gui::string_construct_copy(&list->label,
                                   &lifecycle::construction::unresolved::shared_string);
    } catch (...) {
        family::combo_box_destroy(pick_widget(list));
        throw;
    }
    list->default_value = list->binding = 0;
    pick_widget(list)->methods = pick_list_methods.data();
    return list;
}

void destroy_pick_list(PickList *list) {
    pick_widget(list)->methods = pick_list_methods.data();
    gui::string_destruct(&list->label);
    family::combo_box_destroy(pick_widget(list));
}

static PickList *__fastcall delete_pick_list(PickList *list, void *, Dword flags) {
    destroy_pick_list(list);
    if (flags & 1)
        gui::release(list);
    return list;
}

const char *__cdecl pick_list_class_key() {
    return "PC_PickList";
}

static const char *__fastcall pick_list_name(PickList *, void *) {
    return "PC_PickList";
}

static Dword __fastcall pick_list_is_type(PickList *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(pick_list_class_key) ||
           type == reinterpret_cast<const void *>(family::combo_box_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall pick_list_is_name(PickList *, void *, const char *name) {
    return !std::strncmp(name, "PC_PickList", 12) || !std::strncmp(name, "GComboBox", 10) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

LapList *construct_lap_list(LapList *list, void *parent) {
    construct_pick_list(&list->base, parent);
    try {
        gui::string_construct_copy(&list->custom_label,
                                   &lifecycle::construction::unresolved::shared_string);
    } catch (...) {
        destroy_pick_list(&list->base);
        throw;
    }
    lap_widget(list)->methods = lap_list_methods.data();
    return list;
}

void destroy_lap_list(LapList *list) {
    lap_widget(list)->methods = lap_list_methods.data();
    gui::string_destruct(&list->custom_label);
    destroy_pick_list(&list->base);
}

static LapList *__fastcall delete_lap_list(LapList *list, void *, Dword flags) {
    destroy_lap_list(list);
    if (flags & 1)
        gui::release(list);
    return list;
}

const char *__cdecl lap_list_class_key() {
    return "PC_LapList";
}

static const char *__fastcall lap_list_name(LapList *, void *) {
    return "PC_LapList";
}

static Dword __fastcall lap_list_is_type(LapList *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(lap_list_class_key) ||
           type == reinterpret_cast<const void *>(pick_list_class_key) ||
           type == reinterpret_cast<const void *>(family::combo_box_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall lap_list_is_name(LapList *, void *, const char *name) {
    return !std::strncmp(name, "PC_LapList", 11) || !std::strncmp(name, "PC_PickList", 12) ||
           !std::strncmp(name, "GComboBox", 10) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 57> pick_list_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_pick_list), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::pick_list_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::pick_list_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::pick_list_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_pick_list), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_pick_list), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::text_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::pick_list_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_configure_geometry), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_pick_list), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_character), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_move), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_edit_line), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::reject_edit_character), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::begin_edit_session), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_51), // 51.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::validate_combo_text), // 52.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_53), // 53.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_edit_caret), // 54.
    reinterpret_cast<Method>(::nfs::application::frontend::commit_pick_list), // 55.
    reinterpret_cast<Method>(::nfs::application::frontend::fit_pick_list), // 56.
};

const std::array<Method, 57> lap_list_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_lap_list), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::lap_list_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::lap_list_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::lap_list_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_pick_list), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_pick_list), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::text_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::lap_list_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_configure_geometry), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_lap_list), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_character), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_move), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::combo_box_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_edit_line), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::reject_edit_character), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::begin_edit_session), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_51), // 51.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::validate_combo_text), // 52.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::edit_text_item_operation_53), // 53.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_edit_caret), // 54.
    reinterpret_cast<Method>(::nfs::application::frontend::commit_lap_list), // 55.
    reinterpret_cast<Method>(::nfs::application::frontend::fit_pick_list), // 56.
};

static void *create_pick_list(void *parent) {
    auto *list = static_cast<PickList *>(gui::allocate_flags(sizeof(PickList), 0, "PC_PickList"));
    if (!list)
        return nullptr;
    try {
        return construct_pick_list(list, parent);
    } catch (...) {
        gui::release_flags(list, 0, "PC_PickList");
        throw;
    }
}

static void *create_lap_list(void *parent) {
    auto *list = static_cast<LapList *>(gui::allocate_flags(sizeof(LapList), 0, "PC_LapList"));
    if (!list)
        return nullptr;
    try {
        return construct_lap_list(list, parent);
    } catch (...) {
        gui::release(list);
        throw;
    }
}
static ClassRecord pick_list_class, lap_list_class;

static void unregister_pick_list() {
    unregister_class(&pick_list_class);
}

static void register_pick_list() {
    register_class(&pick_list_class, "PC_PickList", create_pick_list, "GComboBox");
    std::atexit(unregister_pick_list);
}

static void unregister_lap_list() {
    unregister_class(&lap_list_class);
}

static void register_lap_list() {
    register_class(&lap_list_class, "PC_LapList", create_lap_list, "PC_PickList");
    std::atexit(unregister_lap_list);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_lap_list, ".CRT$XCU_NFS_006900C0",
                       register_lap_list);
NFS_KERNEL_INITIALIZER(startup_register_pick_list, ".CRT$XCU_NFS_00690118",
                       register_pick_list);
} // namespace
} // namespace nfs::application::frontend
