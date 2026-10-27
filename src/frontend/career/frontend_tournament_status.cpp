#include "../screens/frontend_menu_roots.hpp"
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;
Dword tournament_page_state;
static void queue_tournament_page(const char *name, const char *type) {
    pending::LocalFrontendString page(name), kind(type);
    pending::enqueue_screen(system(96), page.release(), kind.release());
}

TournamentStatus *construct_tournament_status(TournamentStatus *screen, void *parent) {
    gui::widgets::pending::construct_interface(screen, parent);
    screen->base.methods = tournament_status_methods.data();
    try {
        application::prepare_race(system(92), 0);
        if (tournament_page_state == 4 || tournament_page_state == 5)
            tournament_page_state = 0;
        void *progress = pending::active_event_progress(system(72));
        if (pending::event_complete_progress(progress)) {
            if (!tournament_page_state)
                tournament_page_state = 1;
            else if (tournament_page_state == 2)
                tournament_page_state = 3;
        } else if (!pending::event_stage(pending::active_event_progress(system(72))))
            tournament_page_state = 0;
        else if (!tournament_page_state)
            tournament_page_state = 1;
    } catch (...) {
        destroy_interface(&screen->base);
        throw;
    }
    return screen;
}

void destroy_tournament_status(TournamentStatus *screen) {
    screen->base.methods = tournament_status_methods.data();
    destroy_interface(&screen->base);
}

void __fastcall tournament_status_child(TournamentStatus *screen, void *, Widget *sender,
                                        Dword event, Dword, Dword modifiers) {
    if (event != 43 || std::uint8_t(modifiers))
        return;
    Widget *next = track_child(screen, "Next", family::button_class_key),
           *back = track_child(screen, "Back", family::button_class_key);
    if (sender == next) {
        void *progress = pending::active_event_progress(system(72));
        if (pending::event_complete_progress(progress)) {
            if (tournament_page_state == 3) {
                tournament_page_state = 0;
                invoke<void>(system(92), 4);
                pop_frontend_screen(system(96));
                pop_frontend_screen(system(96));
                family::finish_modal(&screen->base, 5, 0);
                return;
            }
            tournament_page_state = 3;
            queue_tournament_page("TourneyStatus", "TourneyStatus");
        } else if (pending::event_stage(pending::active_event_progress(system(72))) &&
                   tournament_page_state == 1) {
            tournament_page_state = 2;
            queue_tournament_page("TourneyStatus", "TourneyStatus");
        } else {
            tournament_page_state = 4;
            queue_tournament_page("CarSelect", "CarSelectScreen");
        }
        family::finish_modal(&screen->base, 4, 0);
    } else if (sender == back) {
        switch (tournament_page_state) {
        case 0:
        case 1:
            tournament_page_state = 5;
            invoke<void>(system(92), 4);
            break;
        case 2:
        case 3:
            tournament_page_state = 0;
            break;
        case 5:
            invoke<void>(system(92), 4);
            break;
        }
        pop_frontend_screen(system(96));
        family::finish_modal(&screen->base, 5, 0);
    }
}

Dword __fastcall tournament_status_key(TournamentStatus *screen, void *, Dword key, Dword modifiers) {
    return std::uint8_t(modifiers)
               ? 0
               : reinterpret_cast<Dword(__fastcall *)(void *, void *, Dword, Dword)>(
                     interface_methods[32])(screen, nullptr, key, modifiers);
}

void pop_frontend_screen(void *frontend) {
    auto *queue = game::runtime::at<gui::List *>(frontend, 48);
    if (queue) {
        if (void *command = gui::events::pop_front(queue))
            invoke<void>(command, 0, Dword(1));
        if (!gui::events::list_count(queue)) {
            invoke<void>(queue, 0, Dword(1));
            game::runtime::at<void *>(frontend, 48) = nullptr;
        }
    }
}
} // namespace nfs::application::frontend
