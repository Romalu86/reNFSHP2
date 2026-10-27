#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_toggle_widget.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

static const char *__fastcall profile_toggle_name(Widget *, void *) {
    return "ToggleButton";
}

static Dword __fastcall profile_toggle_is_type(Widget *widget, void *, const void *type) {
    return type == reinterpret_cast<const void *>(profiles::profile_list_class_key) ||
           reinterpret_cast<Dword(__thiscall *)(Widget *, const void *)>(
               family::toggle_button_methods[2])(widget, type);
}

static Dword __fastcall profile_toggle_is_name(Widget *widget, void *, const char *name) {
    return !std::strncmp(name, "ToggleButton", 13) ||
           reinterpret_cast<Dword(__thiscall *)(Widget *, const char *)>(
               family::toggle_button_methods[3])(widget, name);
}

static ProfileToggle *construct_profile_toggle(ProfileToggle *widget, void *parent) {
    gui::widgets::pending::construct_toggle_button(widget, parent);
    widget->populate_from_list = 1;
    try {
        gui::string_construct(&widget->list_name);
        widget->default_value = widget->binding = 0;
        widget->button.base.base.methods = profile_toggle_methods.data();
        widget->items.methods = profile_toggle_items_methods.data();
    } catch (...) {
        family::toggle_button_destroy(&widget->button.base.base);
        throw;
    }
    return widget;
}

static void destroy_profile_toggle(ProfileToggle *widget) {
    widget->button.base.base.methods = profile_toggle_methods.data();
    widget->items.methods = profile_toggle_items_methods.data();
    gui::string_destruct(&widget->list_name);
    family::toggle_button_destroy(&widget->button.base.base);
}

static ProfileToggle *__fastcall delete_profile_toggle(ProfileToggle *widget, void *, Dword flags) {
    destroy_profile_toggle(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static ProfileToggle *__fastcall delete_profile_toggle_items(family::ListItems *items, void *,
                                                             Dword flags) {
    return delete_profile_toggle(
        reinterpret_cast<ProfileToggle *>(game::runtime::bytes(items) - 312), nullptr, flags);
}

const std::array<Method, 14> profile_toggle_items_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_profile_toggle_items), // 0.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::clear_list_items), // 1.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::clear_list_cell), // 2.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::append_list_row), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::insert_list_row), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::erase_list_row), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::reorder_list_rows), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::copy_list_cell), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::append_list_cell), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::list_cell), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::select_single_row), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::select_row_range), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::select_row), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::clear_list_selection), // 13.
};

const std::array<Method, 52> profile_toggle_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_profile_toggle), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::profile_toggle_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::profile_toggle_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::profile_toggle_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_profile_toggle), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_profile_toggle), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::profile_toggle_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::toggle_button_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_profile_toggle), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::profile_toggle_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::toggle_button_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::set_button_checked), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_visual_state), // 48.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::advance_toggle_button), // 49.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_item_operation_50), // 50.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::button_set_checked_event), // 51.
};

static void *create_profile_toggle(void *parent) {
    auto *storage =
        static_cast<ProfileToggle *>(gui::allocate_flags(sizeof(ProfileToggle), 0, "ToggleButton"));
    if (!storage)
        return nullptr;
    try {
        return construct_profile_toggle(storage, parent);
    } catch (...) {
        gui::release(storage);
        throw;
    }
}
static ClassRecord profile_toggle_class;

static void unregister_profile_toggle() {
    unregister_class(&profile_toggle_class);
}

static void register_profile_toggle() {
    register_class(&profile_toggle_class, "ToggleButton", create_profile_toggle, "GToggleButton");
    std::atexit(unregister_profile_toggle);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_profile_toggle, ".CRT$XCU_NFS_00690148",
                       register_profile_toggle);
} // namespace
} // namespace nfs::application::frontend
