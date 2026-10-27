#include "nfs/audio/music/music_clients.hpp"
#include "nfs/gui/events/gui_tasks.hpp"
#include "nfs/audio/backend/sound_ownership.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/core/timing/timer.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;
static void release_music_stream(MusicPlayer *player) {
    if (configuration<Dword>(audio_layout::audio_enabled) && !disable_music && player->buffer) {
        void *memory = player->buffer;
        sound::lock();
        sound::streaming::clear_processor(library().stream, 31);
        sound::unlock();
        sound::streaming::destroy_stream(library().stream);
        kernel::memory_free(memory);
        player->buffer = nullptr;
    }
}

void select_music(MusicPlayer *player, int selection) {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    if (selection < 0) {
        if (option<int>(audio_layout::music_source) == -3)
            return;
        if (selection == -2 && library().selection != -1 && !network::session_active())
            return;
        if (!disable_music) {
            initialize_music_stream(player);
            library().selection = selection == -2 ? -2 : -1;
            begin_track(player, -1);
        }
    } else {
        initialize_music_stream(player);
        begin_track(player, selection);
    }
    gui::register_task(poll_music);
}

void fade_out_music(MusicPlayer *player, Dword duration) {
    if (option<int>(audio_layout::music_source) == -3)
        return;
    if (library().playing == 1) {
        sound::lock();
        runtime::race_audio::fade_stream_volume(library().stream, int(duration), 0);
        sound::unlock();
        int volume;
        do {
            sound::lock();
            volume = sound::streaming::stream_volume(library().stream);
            sound::unlock();
        } while (volume > 0);
    }
    gui::unregister_task(poll_music);
    release_music_stream(player);
    library().playing = 0;
}
extern Dword update_window_audio(Dword, Dword);
extern void restart_game_audio();
extern void load_core_banks();
} // namespace nfs::game::audio
namespace nfs::lifecycle::table_entries {
using namespace game::audio;
using namespace game::audio::state;

void __fastcall initialize_audio_client(System *system, void *) {
    initialize_configuration();
    if (sound::field<int>(manager->slots[18], 2360) == 3 && !has_multichannel_audio())
        sound::field<int>(manager->slots[18], 2360) = 1;
    initialize_audio_device();
    reinterpret_cast<MusicPlayer *>(system)->buffer = nullptr;
    kernel::task_add(update_window_audio, 0, 3);
}

void __fastcall shutdown_audio_client(System *system, void *) {
    if (option<int>(audio_layout::music_source) == -3) {
        stop_cd_track();
        gui::unregister_task(poll_music);
        library().playing = 0;
    } else if (configuration<Dword>(audio_layout::audio_enabled))
        release_music_stream(reinterpret_cast<MusicPlayer *>(system));
    if (configuration<Dword>(audio_layout::audio_enabled))
        sound::backend::shutdown_audio();
    kernel::task_remove(update_window_audio);
}

void __fastcall leave_audio_frontend(System *system, void *) {
    play_frontend_event(int(network::rules::random() % 3) + 10, 127);
    if (option<int>(audio_layout::music_source) != -3)
        fade_out_music(reinterpret_cast<MusicPlayer *>(system), quick_fade ? 1 : 3000);
    release_frontend_bank();
}

void __fastcall enter_audio_frontend(System *, void *) {
    configuration<Dword>(audio_layout::music_muted) = 0;
    load_frontend_bank();
}

void __fastcall unload_audio_level(System *system, void *) {
    release_game_audio();
    if (option<int>(audio_layout::music_source) != -3) {
        release_music_stream(reinterpret_cast<MusicPlayer *>(system));
        gui::unregister_task(poll_music);
        library().playing = 0;
    }
}

void __fastcall load_audio_level(System *system, void *) {
    initialize_race_resources();
    initialize_race_listeners();
    load_race_banks_if_ready();
    if (configuration<Dword>(audio_layout::audio_enabled) && option<int>(audio_layout::music_source) != -3) {
        if (library().selection != -1 && !network::session_active())
            return;
        if (!disable_music) {
            initialize_music_stream(reinterpret_cast<MusicPlayer *>(system));
            library().selection = -2;
            begin_track(reinterpret_cast<MusicPlayer *>(system), -1);
        }
        gui::register_task(poll_music);
    }
}
// JMP to the restart body.
void __fastcall reset_audio_client(System *, void *) {
    restart_game_audio();
}
// JMP to core-bank loading.
void __fastcall load_audio_banks(System *, void *) {
    load_core_banks();
}
} // namespace nfs::lifecycle::table_entries
