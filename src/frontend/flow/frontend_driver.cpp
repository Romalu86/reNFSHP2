#include "frontend_runtime.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
namespace nfs::application::pending {
namespace {
void enqueue_named_screen(void *frontend, const char *name, const char *type) {
    LocalFrontendString copied_type(type), copied_name(name);
    enqueue_screen(frontend, copied_name.release(), copied_type.release());
}
int run_named_screen(void *frontend, const char *name, const char *type) {
    LocalFrontendString copied_type(type), copied_name(name);
    return run_screen_queue(frontend, copied_name.release(), copied_type.release());
}
void copy_player_network_name(Dword player) {
    LocalFrontendString name;
    copy_preference_name(player_preferences(system(72), player), &name.value);
    set_preference_network_name(player_preferences(system(72), player), &name.value);
}
void clear_navigation_queue(void *frontend) {
    if (auto *queue = at<gui::List *>(frontend, 48)) {
        gui::events::clear_list(queue);
        if (auto *current = at<gui::List *>(frontend, 48))
            gui::destroy(reinterpret_cast<gui::Object *>(current));
        at<gui::List *>(frontend, 48) = nullptr;
    }
}
} // namespace

int run_frontend(void *frontend, int state) {
    if (!at<void *>(frontend, 4)) {
        if (void *race_view = at<void *>(frontend, 8)) {
            invoke(race_view, 248 / 4);
            gui::destroy(static_cast<gui::Object *>(race_view));
            at<void *>(frontend, 8) = nullptr;
        }
        const Dword previous_pool = lifecycle::ui_pool;
        lifecycle::ui_pool = lifecycle::track_pool;
        at<Dword>(frontend, 52) = previous_pool;
        create_frontend_view(frontend, 1);
    }
    network::packets::pending::set_game_rule(system(72), 13, 1);
    bool copy_names = true;
    if (initial_frontend_entry) {
        initial_frontend_entry = 0;
        if (network::session_active()) {
            LocalFrontendString placeholder;
            gui::string_reference(&placeholder.value, 2064);
            set_preference_name(player_preferences(system(72), 0), &placeholder.value);
            set_preference_network_name(player_preferences(system(72), 0), &placeholder.value);
            services::pending::configure_client(system(112), 0);
            gui::string_reference(&placeholder.value, 454);
            set_preference_name(player_preferences(system(72), 1), &placeholder.value);
            set_preference_network_name(player_preferences(system(72), 1), &placeholder.value);
            services::pending::configure_client(system(112), 1);
            return 2;
        }
        if (exit_after_network_race())
            return 6;
        copy_names = !initial_frontend_entry;
    }
    if (copy_names) {
        copy_player_network_name(0);
        copy_player_network_name(1);
    }
    initial_frontend_entry = 0;
    if (state == 25) {
        enqueue_named_screen(frontend, "Main", "MainMenu");
        if (Byte(services::special_race_mode(system(72))))
            enqueue_named_screen(frontend,
                                 at<Dword>(system(72), 148) == 2 ? "HotPursuit" : "Championship",
                                 "TreeScreen");
        return run_named_screen(frontend, "TourneyStatus", "TourneyStatus");
    }
    if (state == 26 || state == 27 || state == 29) {
        clear_navigation_queue(frontend);
        enqueue_named_screen(frontend, "Main", "MainMenu");
        if (state == 26)
            return run_named_screen(frontend, "HotPursuit", "TreeScreen");
        if (state == 27)
            return run_named_screen(frontend, "Championship", "TreeScreen");
        return run_named_screen(frontend, "Multiplayer", "MultiplayerScreen");
    }
    if (launch::field<Dword>(8627684))
        return run_named_screen(frontend, "TrackList", "GInterface");

    clear_navigation_queue(frontend);
    return run_named_screen(frontend, at<Dword>(system(72), 2312) == 1 ? "MainF" : "Main",
                            "MainMenu");
}
} // namespace nfs::application::pending
