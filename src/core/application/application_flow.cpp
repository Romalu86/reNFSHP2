#include "nfs/core/application/application.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/resources/files/filesystem.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/timing/timer.hpp"
#include <cstdio>
#include <cstring>
#include <intrin.h>

namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::application {
using game::runtime::at;
using game::runtime::system;

int run_outer_flow() {
    char stats_path[252], movie_path[260];
    int screen = 0;
    std::sprintf(stats_path, "%s\\stats.ini", graphics::startup::configuration_path());
    files::erase_sync(stats_path, 100);
    hide_cursor();
    if (!launch::field<Dword>(8627736) && !launch::field<Dword>(8627744)) {
        select_movie(movie_path, 1);
        pending::play_movie(movie_path, 0, 1);
        const auto delay = static_cast<std::int32_t>(tick_frequency() * 7);
        wait_ticks(delay / 2);
    }
    at<Byte>(system(96), 12) = 1;
    lifecycle::enter_frontend(lifecycle::manager);
    screen = pending::run_initial_frontend(system(96));
    lifecycle::leave_frontend(lifecycle::manager);
    const bool server = launch::field<Dword>(8627736) != 0;
    at<Byte>(system(96), 12) = 0;
    bool frontend_entered = false;
    if (!server && !launch::field<Dword>(8627744)) {
        select_movie(movie_path, 0);
        pending::play_movie(movie_path, 1, 1);
        do {
            if (!frontend_entered) {
                lifecycle::enter_frontend(lifecycle::manager);
                frontend_entered = true;
            }
            screen =
                launch::field<Dword>(8627776) ? 3 : pending::run_network_frontend(system(96));
            if (screen != 8)
                break;
            if (frontend_entered) {
                lifecycle::leave_frontend(lifecycle::manager);
                frontend_entered = false;
            }
            select_movie(movie_path, 0);
            hide_cursor();
            pending::play_movie(movie_path, 1, 1);
            show_cursor(1);
        } while (screen == 8);
    }
    seed_effect_random(static_cast<Dword>(__rdtsc()));
    for (;;) {
        if (screen == 6)
            return 0;
        do {
            if (!frontend_entered) {
                lifecycle::enter_frontend(lifecycle::manager);
                frontend_entered = true;
            }
            if (!launch::field<Dword>(8627680)) {
                show_cursor(1);
                screen = pending::run_frontend(system(96), screen);
                show_cursor(0);
            }
            if (frontend_entered) {
                lifecycle::leave_frontend(lifecycle::manager);
                frontend_entered = false;
            }
            if (screen != 8)
                break;
            if (game::audio::state::option<int>(audio_layout::music_source) == -3) {
                game::audio::stop_cd_track();
                gui::unregister_task(game::audio::poll_music);
                game::audio::state::library().playing = 0;
            }
            select_movie(movie_path, 0);
            hide_cursor();
            pending::play_movie(movie_path, 1, 1);
            show_cursor(1);
            if (game::audio::state::option<int>(audio_layout::music_source) == -3) {
                game::audio::play_cd_track(game::audio::next_cd_track());
                gui::register_task(game::audio::poll_music);
            }
        } while (screen == 8);
        if (screen == 6)
            return 0;
        const bool same_display = graphics::startup::modes_match(&graphics::startup::settings) != 0;
        if (!same_display)
            pending::switch_display(0);
        const Dword retry = 0;
        do {
            files::cache::application::pause();
            seed_effect_random(static_cast<Dword>(__rdtsc()));
            at<Dword>(system(96), 36) = retry;
            pending::create_loading_screen(system(96));
            game::runtime::reset_step_state();
            if (!std::memcmp(reinterpret_cast<const Byte *>(system(40)) + 140, "Invalid", 8))
                network::packets::pending::select_track(system(40), 0);
            prepare_race(system(92), 0);
            lifecycle::load_level(lifecycle::manager);
            notify_race_loaded(system(92));
            close_loading_screen(system(96));
            ini::flush_cache();
            screen = run_race_screen(system(96));
            ini::flush_cache();
            if (network::session_active()) {
                if (screen == 19)
                    screen = 29;
            } else if (screen == 19) {
                if (at<int>(system(72), 152) == 2)
                    screen = 25;
                else if (at<int>(system(72), 148) == 2)
                    screen = 26;
                else if (at<int>(system(72), 148) == 4)
                    screen = 27;
            }
            notify_race_unloading(system(92));
            lifecycle::unload_level(lifecycle::manager);
            notify_race_unloaded(system(92));
            files::archives::clear();
            files::cache::application::start();
        } while (screen == 24);
        pending::finish_network_race_session(system(104));
        if (exit_after_network_race())
            screen = 6;
        if (!same_display) {
            if (screen == 6)
                return 0;
            pending::switch_display(1);
        }
    }
}


void select_movie(char *path, Byte logo) {
    const char *movie = "logo";
    if (!logo) {
        ++movie_counter;
        movie = "hp2";
    }
    static const char *const languages[] = {"E.MAD",  "F.MAD", "G.MAD", "I.MAD",
                                            "SP.MAD", "S.MAD", "K.MAD"};
    const auto directory = [] { return reinterpret_cast<const char *>(system(4)) + 1824; };
    std::sprintf(path, "%s%s%s", directory(), movie, languages[at<int>(system(72), 2312)]);
    if (!files::file::exists(path)) {
        for (const char *language : languages) {
            std::sprintf(path, "%s%s%s", directory(), movie, language);
            if (files::file::exists(path))
                return;
        }
        std::sprintf(path, "%s%s%s", directory(), movie, languages[0]);
    }
}

// Shared six-word state used by effects, network rules and audio.
void seed_effect_random(Dword seed) {
    const Dword offsets[] = {4063039062u, 2284922601u, 3324304687u, 117621916, 2654289789u, 1876900708};
    for (unsigned i = 0; i < 6; ++i)
        network::rules::random_state[i] = seed + offsets[i];
}


Dword exit_after_network_race() {
    return (launch::field<Dword>(8627736) || launch::field<Dword>(8627744)) &&
           launch::network_requested;
}
} // namespace nfs::application
