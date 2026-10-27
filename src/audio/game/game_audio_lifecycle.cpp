#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/audio/backend/sound_ownership.hpp"
#include <cstdio>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;

void reset_game_audio() {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    for (unsigned event = 0; event < 124; ++event) {
        channels()[event].voice = channels()[event].event = Dword(-1);
        channels()[event + 124].voice = channels()[event + 124].event = Dword(-1);
    }
    option<Dword>(audio_layout::player_engine_level) = option<Dword>(audio_layout::second_player_engine_level) = 0;
    for (unsigned event = 0; event < 124; ++event) {
        channel_variation[event] = 64;
        channel_variation[124 + event] = 64;
    }
    configuration<int>(audio_layout::reverb_preset) = -1;
    configuration<Dword>(1684) = 0;
    configuration<Dword>(audio_layout::music_muted) = 0;
    void *cars = lifecycle::manager->slots[17];
    for (Dword index = 0; index < sound::field<Dword>(cars, 8); ++index) {
        Dword offset = 20 + index * 4;
        if (sound::field<void *>(cars, offset) && offset < 116)
            reset_car_audio(sound::field<void *>(cars, offset));
        cars = lifecycle::manager->slots[17];
    }
    sound::set_master_volume(120);
    option<Dword>(152) = 4;
    option<Dword>(audio_layout::last_announced_lap) = option<Dword>(180) = 0;
    runtime::race_audio::pending::reset_listener(0);
    runtime::race_audio::pending::reset_listener(1);
    reset_engine_state();
}

void initialize_race_resources() {
    configuration<Dword>(12) = configuration<Dword>(8) = 0;
    reset_audio_world();
    reset_game_audio();
    load_reverb_resources();
}

void release_race_banks() {
    stop_race_channels();
    for (unsigned bank = 13; bank < 47; ++bank) {
        if (banks()[bank].handle != -1)
            sound::release_bank(banks()[bank].handle);
        if (banks()[bank].data) {
            banks()[bank].handle = -1;
            kernel::memory_free(banks()[bank].data);
            banks()[bank].data = nullptr;
        }
    }
    if (sound::field<Byte>(effects_storage, 0)) {
        pending::release_track_audio();
        sound::field<Byte>(effects_storage, 0) = 0;
    }
}

void release_game_audio() {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    if (!configuration<Dword>(audio_layout::music_muted))
        for (int volume = 127; volume >= 0; --volume)
            sound::set_master_volume(Byte(volume));
    sound::stop_all();
    sound::release_bank(-1);
    for (unsigned bank = 0; bank < 49; ++bank)
        if (banks()[bank].data) {
            banks()[bank].handle = -1;
            kernel::memory_free(banks()[bank].data);
            banks()[bank].data = nullptr;
        }
    if (configuration<Dword>(1684) && configuration<Dword>(audio_layout::audio_enabled)) {
        configuration<Dword>(audio_layout::reverb_preset) = 0;
        sound::backend::configure_effect_groups(0, 0, 0, Dword(-1), Dword(-1));
        sound::backend::set_effect_group_flags(0, 0);
    }
    sound::set_master_volume(120);
    if (sound::field<Byte>(effects_storage, 0)) {
        pending::release_track_audio();
        sound::field<Byte>(effects_storage, 0) = 0;
    }
    for (unsigned index = 0; index < 7; ++index)
        if (void *data = sound::field<void *>(effects_storage, 4 + index * 4)) {
            kernel::memory_free(data);
            sound::field<void *>(effects_storage, 4 + index * 4) = nullptr;
        }
    configuration<Dword>(12) = configuration<Dword>(8) = 0;
}

void restart_game_audio() {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    configuration<Dword>(audio_layout::music_muted) = 0;
    sound::set_master_volume(0);
    stop_game_channels();
    for (unsigned volume = 0; volume < 120; ++volume)
        sound::set_master_volume(Byte(volume));
    if (configuration<Dword>(1684) && configuration<Dword>(audio_layout::audio_enabled)) {
        configuration<Dword>(audio_layout::reverb_preset) = 0;
        sound::backend::configure_effect_groups(0, 0, 0, Dword(-1), Dword(-1));
        sound::backend::set_effect_group_flags(0, 0);
    }
    if (!configuration<Dword>(8))
        restore_music_volume();
    reset_game_audio();
}

void load_core_banks() {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    for (unsigned index = 0; index < 49; ++index)
        banks()[index].handle = -1;
    char filename[260];
    std::strcpy(filename, path(1304));
    std::strcat(filename, "Gen");
    load_bank(filename, 2, lifecycle::audio_pool | 1024);
    std::strcpy(filename, runtime::track_file("track"));
    load_bank(filename, 3, lifecycle::audio_pool | 1024);
    std::strcpy(filename, path(1304));
    std::strcat(filename, "siren");
    load_bank(filename, 5, lifecycle::audio_pool | 1024);
}

void load_race_banks_if_ready() {
    if (network::session_active() &&
        sound::field<std::int16_t>(lifecycle::manager->slots[26], 10) < 4)
        return;
    pending::load_race_banks();
}
} // namespace nfs::game::audio
