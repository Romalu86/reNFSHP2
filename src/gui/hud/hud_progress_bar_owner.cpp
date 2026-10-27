#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_display.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;

static ProgressBar *construct_progress_bar(ProgressBar *widget, void *parent) {
    gui::widgets::pending::construct_static_image(widget, parent);
    widget->pair = nullptr;
    widget->fraction = 1;
    reinterpret_cast<Widget *>(widget)->methods = progress_bar_methods.data();
    try {
        widget->timer = gui::create_widget_timer(reinterpret_cast<Widget *>(widget)->id, .25f);
    } catch (...) {
        family::static_image_destroy(reinterpret_cast<Widget *>(widget));
        throw;
    }
    return widget;
}

static void destroy_progress_bar(ProgressBar *widget) {
    reinterpret_cast<Widget *>(widget)->methods = progress_bar_methods.data();
    try {
        gui::remove_widget_timer(widget->timer);
    } catch (...) {
        family::static_image_destroy(reinterpret_cast<Widget *>(widget));
        throw;
    }
    family::static_image_destroy(reinterpret_cast<Widget *>(widget));
}

static ProgressBar *__fastcall delete_progress_bar(ProgressBar *widget, void *, Dword flags) {
    destroy_progress_bar(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const char *__cdecl progress_bar_class_key() {
    return "ProgressBar";
}

static const char *__fastcall progress_bar_name(ProgressBar *, void *) {
    return "ProgressBar";
}

static Dword __fastcall progress_bar_is_type(ProgressBar *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(progress_bar_class_key) ||
           type == reinterpret_cast<const void *>(family::static_image_class_key) ||
           type == reinterpret_cast<const void *>(family::image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall progress_bar_is_name(ProgressBar *, void *, const char *name) {
    return !std::strncmp(name, "ProgressBar", 12) || !std::strncmp(name, "GStaticImage", 13) ||
           !std::strncmp(name, "GImage", 7) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 48> progress_bar_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_progress_bar), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::progress_bar_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::progress_bar_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::progress_bar_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_load), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_save), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_push_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::draw_progress_bar), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::request_general_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::progress_bar_timer), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_resource), // 47.
};

void *create_progress_bar(void *parent) {
    auto *widget =
        static_cast<ProgressBar *>(gui::allocate_flags(sizeof(ProgressBar), 0, "ProgressBar"));
    if (!widget)
        return nullptr;
    try {
        return construct_progress_bar(widget, parent);
    } catch (...) {
        gui::release(widget);
        throw;
    }
}
} // namespace nfs::application::frontend::hud
