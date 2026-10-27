#include "frontend_simulation.hpp"
namespace nfs::application::pending {
gui::String *construct_integer_text(gui::String *, int);
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void __fastcall initialize_end_game(Widget *screen, void *) {
    family::dialog_initialize(screen, nullptr);
    at<Dword>(screen, 272) = 0;
    show_cursor(1);
    if (auto *total = static_cast<Widget *>(pending::find_child(screen, "<TOTAL>"))) {
        gui::String text;
        pending::construct_integer_text(&text, at<int>(system(72), 9728));
        try {
            invoke<void>(total, 7, &text);
        } catch (...) {
            gui::string_destruct(&text);
            throw;
        }
        gui::string_destruct(&text);
    }
    if (at<Dword>(system(72), 152) == 2 &&
        !pending::event_complete_progress(pending::active_event_progress(system(72)))) {
        if (auto *button = static_cast<Widget *>(pending::find_child(screen, "B3"))) {
            {
                pending::LocalFrontendString text;
                gui::string_reference(&text.value, 476);
                invoke<void>(button, 7, &text.value);
            }
            pending::set_widget_tab_order(button, 0);
        }
    }
}
 
void __fastcall release_end_game(Widget *screen, void *) {
    clear_layout_request(screen, nullptr);
}

void __fastcall tick_end_game(Widget *screen, void *, Dword ticks) {
    family::dialog_tick(screen, nullptr, ticks);
    if (!screen->modal_result) {
        if (at<Dword>(system(96), 36) == 23) {
            game::audio::stop_race_channels();
            family::finish_modal(screen, at<Dword>(system(96), 36), 0);
        } else {
            game::advance();
        }
    }
}

void __fastcall draw_end_game(Widget *, void *, void *engine) {
    invoke<void>(engine, 56, Dword(6), Dword(0), Dword(0));
    draw_race_frame();
    invoke<void>(engine, 56, Dword(5), Dword(0), Dword(0));
}
} // namespace nfs::application::frontend
