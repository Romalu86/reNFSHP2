#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_simulation.hpp"
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

static void destroy_end_game(Widget *screen) {
    screen->methods = end_game_methods.data();
    family::dialog_destroy(screen);
}

static Widget *__fastcall delete_end_game(Widget *screen, void *, Dword flags) {
    destroy_end_game(screen);
    if (flags & 1)
        gui::release(screen);
    return screen;
}

const char *__cdecl end_game_class_key() { return "HP2EndGame"; }

static const char *__fastcall end_game_name(Widget *, void *) { return "HP2EndGame"; }

static Dword __fastcall end_game_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(end_game_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall end_game_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2EndGame", 11) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 49> end_game_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_end_game), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::end_game_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::end_game_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::end_game_is_name), // 3.
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
    reinterpret_cast<Method>(::nfs::application::frontend::draw_end_game), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_end_game), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_end_game), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::release_end_game), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_key_down), // 32.
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

void *create_end_game_dialog(void *parent) {
    auto *screen = static_cast<Widget *>(gui::allocate_flags(288, 0, "HP2EndGame"));
    if (!screen)
        return nullptr;
    try {
        gui::widgets::pending::construct_dialog(screen, parent);
        screen->methods = end_game_methods.data();
        return screen;
    } catch (...) {
        gui::release_flags(screen, 0, "HP2EndGame");
        throw;
    }
}
} // namespace nfs::application::frontend
