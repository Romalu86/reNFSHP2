#include "nfs/core/application/application.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::application {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

int run_race_screen(void *frontend) {
    show_cursor(0);
    if (!at<Dword>(frontend, 8)) {
        pending::clear_frontend_views(frontend);
        lifecycle::ui_pool = at<Dword>(frontend, 52);
        pending::create_frontend_view(frontend, 0);
    }
    int screen = 0;
    while (screen != 6) {
        if (screen == 19 || screen == 16 || screen == 24)
            break;
        at<Dword>(frontend, 36) = 0;
        if (!network::session_active()) {
            switch (screen) {
            case 6:
            case 19:
                break;
            case 18:
                screen = pending::run_replay_controls(system(96));
                break;
            case 20:
                screen = pending::run_simulation_screen(system(96));
                break;
            case 21:
                screen = pending::run_race_results(system(96));
                break;
            case 22:
                screen = pending::run_end_game_menu(system(96));
                break;
            case 28:
                screen = pending::run_pursuit_results(system(96));
                break;
            default:
                screen = pending::run_starting_grid(system(96));
                break;
            }
        } else {
            at<Dword>(system(72), 140) = 0;
            switch (screen) {
            case 6:
            case 19:
            case 24:
                break;
            case 18:
            case 21:
            case 22:
                screen = 19;
                break;
            default:
                invoke(at<void *>(system(92), 4), 68 / 4);
                at<Byte>(at<void *>(system(92), 4), 44) = 0;
                reset_network_race();
                screen = pending::run_simulation_screen(system(96));
                break;
            }
        }
    }
    show_cursor(1);
    return screen;
}

// The RET4 consumes an ignored stack argument.
void prepare_race(void *manager, Dword) {
    if (!at<void *>(manager, 4)) {
        const Dword event = at<Dword>(system(72), 152);
        const Dword mode = at<Dword>(system(72), 148);
        at<Dword>(manager, 4) =
            pending::create_race(event, mode, "m_race", lifecycle::level_pool | 256);
    }
    void *race = at<void *>(manager, 4);
    if (race && !network::session_active())
        invoke(race, 56 / 4, 0u);
    if (!at<void *>(manager, 8))
        at<void *>(manager, 8) = invoke<void *>(race, 136 / 4);
}


void notify_race_loaded(void *manager) {
    if (void *race = at<void *>(manager, 4))
        invoke(race, 64 / 4);
}

void notify_race_unloading(void *manager) {
    if (void *race = at<void *>(manager, 4))
        invoke(race, 76 / 4);
}

void notify_race_unloaded(void *manager) {
    if (void *race = at<void *>(manager, 4))
        invoke(race, 84 / 4);
}

void reset_network_race() {
    pending::stop_force_feedback();
    pending::reset_subsystems(lifecycle::manager);
    pending::reset_race_state();
}

void close_loading_screen(void *frontend) {
    show_cursor(1);
    if (void *screen = at<void *>(frontend, 40)) {
        pending::destroy_screen(screen);
        at<Dword>(frontend, 40) = 0;
    }
}
} // namespace nfs::application

namespace nfs::application::pending {
// Close and destroy both engine views before replacing one.
void clear_frontend_views(void *frontend) {
    for (unsigned offset : {4u, 8u})
        if (void *view = game::runtime::at<void *>(frontend, offset)) {
            game::runtime::invoke(view, 248 / 4);
            gui::destroy(static_cast<gui::Object *>(view));
            game::runtime::at<void *>(frontend, offset) = nullptr;
        }
}
} // namespace nfs::application::pending
