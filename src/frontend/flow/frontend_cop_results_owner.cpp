#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_race_results.hpp"
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

static CopVsPerpResults *construct_cop_results(CopVsPerpResults *screen, void *parent) {
    gui::widgets::pending::construct_dialog(&screen->base, parent);
    screen->base.methods = cop_results_methods.data();
    return screen;
}

static void destroy_cop_results(CopVsPerpResults *screen) {
    screen->base.methods = cop_results_methods.data();
    family::dialog_destroy(&screen->base);
}

static CopVsPerpResults *__fastcall delete_cop_results(CopVsPerpResults *screen, void *, Dword flags) {
    destroy_cop_results(screen);
    if (flags & 1) gui::release(screen);
    return screen;
}

const char *__cdecl cop_results_class_key() { return "HP2CopVsPerpResults"; }

static const char *__fastcall cop_results_name(CopVsPerpResults *, void *) { return "HP2CopVsPerpResults"; }

static Dword __fastcall cop_results_is_type(CopVsPerpResults *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(cop_results_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall cop_results_is_name(CopVsPerpResults *, void *, const char *name) {
    return !std::strncmp(name, "HP2CopVsPerpResults", 20) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 49> cop_results_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_cop_results), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::cop_results_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::cop_results_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::cop_results_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::draw_cop_results), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_cop_results), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_cop_results), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::finish_cop_results), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::cop_results_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
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

void *create_cop_results(void *parent) {
    auto *screen = static_cast<CopVsPerpResults *>(gui::allocate_flags(sizeof(CopVsPerpResults), 0, "HP2CopVsPerpResults"));
    if (!screen) return nullptr;
    try { return construct_cop_results(screen, parent); }
    catch (...) { gui::release_flags(screen, 0, "HP2CopVsPerpResults"); throw; }
}
}
