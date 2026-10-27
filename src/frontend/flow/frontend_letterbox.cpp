#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../screens/frontend_race_dialogs.hpp"
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

static void destroy_letterbox(GeneralWidget *widget) {
    widget->base.methods = letterbox_methods.data();
    destroy_general(widget);
}

static GeneralWidget *__fastcall delete_letterbox(GeneralWidget *widget, void *, Dword flags) {
    destroy_letterbox(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const char *__cdecl letterbox_class_key() {
    return "HP2LetterBox";
}

static const char *__fastcall letterbox_name(Widget *, void *) {
    return "HP2LetterBox";
}

static Dword __fastcall letterbox_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(letterbox_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall letterbox_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2LetterBox", 13) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}
// Complete body is RET4.
static void __fastcall tick_letterbox(Widget *, void *, Dword) {
}

const std::array<Method, 47> letterbox_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_letterbox), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::letterbox_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::letterbox_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::letterbox_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_general), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_general), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::request_general_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_letterbox), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

void *create_letterbox(void *parent) {
    auto *widget = static_cast<GeneralWidget *>(gui::allocate_flags(264, 0, "HP2LetterBox"));
    if (!widget)
        return nullptr;
    try {
        construct_widget(widget, parent);
        widget->base.methods = letterbox_methods.data();
        return widget;
    } catch (...) {
        gui::release_flags(widget, 0, "HP2LetterBox");
        throw;
    }
}
} // namespace nfs::application::frontend
