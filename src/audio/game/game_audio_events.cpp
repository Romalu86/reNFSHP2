#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;

void play_frontend_event(int event, Dword volume) {
    if (configuration<Dword>(audio_layout::audio_enabled)) {
        sound::VoiceParameters parameters;
        sound::initialize_parameters(&parameters);
        sound::field<Byte>(&parameters, 0) =
            Byte(runtime::race_audio::group_volume(127, 6, volume));
        sound::field<std::uint16_t>(&parameters, 8) = 0;
        sound::field<Byte>(&parameters, 1) = 64;
        sound::lock();
        sound::play_bank(banks()[48].handle, event, &parameters);
        sound::unlock();
    }
}

Dword play_bank_event(int bank, int event, std::uint16_t pan, Byte volume, Byte pitch) {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return Dword(-1);
    sound::VoiceParameters parameters;
    sound::initialize_parameters(&parameters);
    if (bank == -1)
        return 0;
    sound::field<Byte>(&parameters, 0) = volume;
    sound::field<std::uint16_t>(&parameters, 8) = pan;
    sound::field<Byte>(&parameters, 1) = pitch;
    sound::lock();
    Dword voice = sound::play_bank(bank, event, &parameters);
    sound::set_voice_effect(voice, 0, 127);
    sound::unlock();
    return voice;
}
static void stop_channels(unsigned count) {
    for (unsigned id = 0; id < count; ++id) {
        runtime::race_audio::stop_event(id);
        runtime::race_audio::stop_event(id | 256);
    }
}

void stop_game_channels() {
    stop_channels(124);
}

void stop_race_channels() {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    stop_channels(112);
    runtime::race_audio::pending::reset_listener(0);
    runtime::race_audio::pending::reset_listener(1);
    runtime::race_audio::pending::reset_environment();
    runtime::race_audio::pending::reset_engine_events();
    sound::lock();
    sound::set_voice_volume(channels()[0].voice, 0);
    sound::unlock();
}

void mute_game_audio() {
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    configuration<Dword>(audio_layout::music_muted) = 1;
    stop_race_channels();
    if (!configuration<Dword>(8))
        restore_music_volume();
    sound::set_master_volume(0);
    if (option<int>(audio_layout::music_source) == -3)
        runtime::race_audio::pending::set_cd_audio_volume(0);
}

void restore_game_volume() {
    if (configuration<Dword>(audio_layout::audio_enabled)) {
        if (!configuration<Dword>(8))
            restore_music_volume();
        if (!configuration<Dword>(audio_layout::music_muted))
            sound::set_master_volume(120);
    }
}
} // namespace nfs::game::audio
