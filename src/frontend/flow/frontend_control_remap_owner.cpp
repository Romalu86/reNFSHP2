#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../../profiles/ui/frontend_profile_controls.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

const char *__cdecl control_remap_class_key() {
    return "HP2ControlRemapDlg";
}

static const char *__fastcall control_remap_name(Widget *, void *) {
    return "HP2ControlRemapDlg";
}

static Dword __fastcall control_remap_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(control_remap_class_key) ||
           type == reinterpret_cast<const void *>(family::msg_box_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall control_remap_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2ControlRemapDlg", 19) || !std::strncmp(name, "GMsgBox", 8) ||
           !std::strncmp(name, "GDialog", 8) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

static void destroy_control_remap(ControlRemapDialog *page) {
    reinterpret_cast<Widget *>(page)->methods = control_remap_methods.data();
    family::msg_box_destroy(&page->base);
}

static ControlRemapDialog *__fastcall delete_control_remap(ControlRemapDialog *page, void *,
                                                           Dword flags) {
    destroy_control_remap(page);
    if (flags & 1)
        gui::release(page);
    return page;
}

const std::array<Method, 49> control_remap_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_control_remap), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::control_remap_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::control_remap_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::control_remap_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_configure_geometry), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_control_remap), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::update_control_remap), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_key_down), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::application::frontend::accept_control_remap), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_button_down), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_accept), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_cancel), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::show_dialog), // 47.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::hide_dialog), // 48.
};

static ControlRemapDialog *construct_control_remap(ControlRemapDialog *page, void *parent) {
    gui::widgets::pending::construct_msg_box(page, parent);
    page->released = page->unused_129 = 0;
    page->elapsed = 0;
    page->input = nullptr;
    page->binding = page->value = 0;
    page->base.methods = control_remap_methods.data();
    std::memset(page->snapshots, 0, sizeof(page->snapshots));
    page->base.flags = 8;
    page->base.layout_flags = 48;
    return page;
}

void *create_control_remap(void *parent) {
    auto *page = static_cast<ControlRemapDialog *>(
        gui::allocate_flags(sizeof(ControlRemapDialog), 0, "HP2ControlRemapDlg"));
    if (!page)
        return nullptr;
    try {
        return construct_control_remap(page, parent);
    } catch (...) {
        gui::release(page);
        throw;
    }
}
} // namespace nfs::application::frontend
