#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_simulation.hpp"
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;

static void destroy_pause_dialog(PauseDialog *screen) {
    screen->base.methods = pause_dialog_methods.data();
    gui::string_destruct(&screen->parent_menu);
    family::dialog_destroy(&screen->base);
}

static PauseDialog *__fastcall delete_pause_dialog(PauseDialog *screen, void *, Dword flags) {
    destroy_pause_dialog(screen);
    if (flags & 1)
        gui::release(screen);
    return screen;
}

const char *__cdecl pause_dialog_class_key() { return "HP2Pause"; }

static const char *__fastcall pause_dialog_name(Widget *, void *) { return "HP2Pause"; }

static Dword __fastcall pause_dialog_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(pause_dialog_class_key) ||
           type == reinterpret_cast<const void *>(family::dialog_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall pause_dialog_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "HP2Pause", 9) || !std::strncmp(name, "GDialog", 8) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 49> pause_dialog_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_pause_dialog), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_dialog_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_dialog_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_dialog_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_dialog_load), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_dialog_save), // 5.
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
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_pause_dialog), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::tick_pause_dialog), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::release_pause_dialog), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::dialog_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::application::frontend::finish_pause_dialog), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::pause_dialog_key_down), // 32.
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

void *create_pause_dialog(void *parent) {
    auto *screen = static_cast<PauseDialog *>(gui::allocate_flags(480, 0, "HP2Pause"));
    if (!screen)
        return nullptr;
    try {
        gui::widgets::pending::construct_dialog(screen, parent);
        gui::string_construct(&screen->parent_menu);
        // Initialize the 43-word settings block without writing slots 1 through 3.
        screen->saved_settings[0] = 0;
        for (unsigned i = 4; i <= 11; ++i)
            screen->saved_settings[i] = 100;
        screen->saved_settings[12] = 1;
        screen->saved_settings[13] = 0;
        for (unsigned i = 14; i <= 27; ++i)
            screen->saved_settings[i] = 1;
        screen->saved_settings[28] = screen->saved_settings[29] = 0;
        for (unsigned i = 30; i <= 34; ++i)
            screen->saved_settings[i] = 100;
        for (unsigned i = 35; i < 43; ++i)
            screen->saved_settings[i] = 0;
        screen->base.methods = pause_dialog_methods.data();
        return screen;
    } catch (...) {
        gui::release_flags(screen, 0, "HP2Pause");
        throw;
    }
}
} // namespace nfs::application::frontend
