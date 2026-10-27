#include "../flow/frontend_simulation.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::system;

void __fastcall initialize_pause_dialog(PauseDialog *screen, void *) {
    family::dialog_initialize(&screen->base, nullptr);
    show_cursor(1);
    std::memcpy(screen->saved_settings, game::runtime::bytes(system(72)) + 2312, 172);
    pending::stop_force_feedback();
    auto hide = [&](const char *name) {
        if (void *child = pending::find_child(screen, name)) {
            set_enabled(static_cast<Widget *>(child), 0);
            pending::set_widget_visible(child, 0);
        }
    };
    if (game::runtime::physics::split_screen()) {
        hide("<JUMPCAM>");
        hide("<360CAM>");
    } else {
        hide("<P2VIB>");
        hide("<SPLIT>");
    }
}

void __fastcall release_pause_dialog(PauseDialog *screen, void *) {
    clear_layout_request(&screen->base, nullptr);
    show_cursor(0);
}

Dword __fastcall finish_pause_dialog(PauseDialog *screen, void *, Dword result, Dword) {
    if (network::session_active()) {
        screen->base.modal_result = result;
        return 1;
    }
    if (result == 2) {
        if (!gui::string_equal(&screen->parent_menu,
                               &lifecycle::construction::unresolved::shared_string))
            return 0;
    }
    if (result == 2 || result == 3)
        gui::string_copy(&at<gui::String>(system(96), 16), &screen->parent_menu);
    screen->base.modal_result = result;
    return 1;
}

Dword __fastcall pause_dialog_key_down(PauseDialog *screen, void *, Dword key, Dword modifiers) {
    return family::dialog_key_down(&screen->base, nullptr, key, modifiers);
}

Dword __fastcall pause_dialog_load(PauseDialog *screen, void *, void *configuration) {
    gui::engine::pending::read_string(configuration, "HP2Pause.mParentMenu", &screen->parent_menu,
                                    &lifecycle::construction::unresolved::shared_string);
    return family::dialog_load(&screen->base, nullptr, Dword(configuration));
}

Dword __fastcall pause_dialog_save(PauseDialog *screen, void *, void *configuration) {
    gui::engine::pending::write_text(configuration, "HP2Pause.mParentMenu",
                                    gui::string_ascii(&screen->parent_menu), nullptr);
    return family::dialog_save(&screen->base, nullptr, Dword(configuration));
}

void __fastcall tick_pause_dialog(PauseDialog *, void *, Dword) {
    pending::stop_force_feedback();
}
} // namespace nfs::application::frontend
