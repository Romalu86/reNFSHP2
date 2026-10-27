#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/audio/backend/sound_ownership.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;
namespace streams = sound::streaming;

void initialize_music_stream(MusicPlayer *player) {
    if (!configuration<Dword>(audio_layout::audio_enabled) || disable_music)
        return;
    sound::VoiceParameters parameters;
    sound::initialize_parameters(&parameters);
    int bytes = streams::file_stream_bytes(1, 300) + 204800;
    player->buffer = kernel::memory_allocate("sndstream", bytes, lifecycle::audio_pool);
    library().stream = streams::create_file_stream(&parameters, 20, 300, player->buffer, bytes);
    if (int(library().stream) < 0) {
        kernel::memory_free(player->buffer);
        player->buffer = nullptr;
    }
    Dword processor[6] = {123,
                         40,
                         31,
                         reinterpret_cast<Dword>(construct_waveform),
                         reinterpret_cast<Dword>(process_waveform),
                         reinterpret_cast<Dword>(destroy_waveform)};
    sound::field<Dword>(wave_processors, 0) = 123;
    sound::lock();
    streams::set_processor(library().stream, processor);
    sound::unlock();
}
static void publish_track(int index, int volume) {
    Library &catalog = library();
    if (!catalog.custom_playlist && index < 50) {
        std::strcpy(catalog.title, catalog.tracks[index].title);
        std::strcpy(catalog.artist, catalog.tracks[index].artist);
        std::strcpy(catalog.album, catalog.tracks[index].album);
        if (volume)
            sound::field<Byte>(&catalog.notify, 0) = 1;
    }
}
// The saved stack parameter is unused; track selection is shared state.
void begin_track(MusicPlayer *player, int) {
    Library &catalog = library();
    if (!configuration<Dword>(audio_layout::audio_enabled) || disable_music || catalog.playing)
        return;
    int volume = runtime::race_audio::group_volume(configuration<Dword>(audio_layout::music_volume),
                                                   catalog.selection == -1 ? 4 : 5, 127);
    catalog.current_track = choose_track();
    if (catalog.current_track == -1)
        return;
    std::strcpy(catalog.current_path, catalog.paths[catalog.current_track]);
    if (!files::file::exists(catalog.current_path))
        return;
    publish_track(catalog.current_track, volume);
    sound::set_master_volume(0);
    player->block = streams::enqueue_file(catalog.stream, 0, catalog.current_path, 0);
    sound::set_master_volume(120);
    sound::lock();
    runtime::race_audio::set_stream_volume(catalog.stream, volume);
    sound::unlock();
    catalog.playing = 1;
    option<int>(audio_layout::music_source) = -4;
    player->last_volume = volume;
}

void update_speech_stream(void *client) {
    if (configuration<Dword>(audio_layout::audio_enabled) && sound::field<Dword>(client, 20)) {
        Dword status[3];
        sound::lock();
        streams::stream_status(sound::field<Dword>(client, 16), status);
        sound::unlock();
        option<Dword>(audio_layout::speech_stream_active) = int(status[0]) > 0;
        sound::field<Dword>(client, 24) = status[0];
    }
}

void update_music(MusicPlayer *player, int context) {
    Library &catalog = library();
    if (!configuration<Dword>(audio_layout::audio_enabled) || !catalog.playing)
        return;
    update_speech_stream(lifecycle::manager->slots[21]);
    int volume = runtime::race_audio::group_volume(configuration<Dword>(audio_layout::music_volume),
                                                   context == -1 ? 4 : 5, 127);
    if (!option<Dword>(audio_layout::speech_stream_active)) {
        if (option<Dword>(audio_layout::music_ducked) && context == -1) {
            sound::lock();
            runtime::race_audio::fade_stream_volume(catalog.stream, 500, volume);
            sound::unlock();
            option<Dword>(audio_layout::music_ducked) = 0;
        }
    } else if (!option<Dword>(audio_layout::music_ducked) && context == -1) {
        sound::lock();
        runtime::race_audio::fade_stream_volume(catalog.stream, 50, volume >> 2);
        sound::unlock();
        option<Dword>(audio_layout::music_ducked) = 1;
    }
    Dword block[4], stream[3];
    sound::lock();
    streams::block_status(player->block, block);
    streams::stream_status(catalog.stream, stream);
    sound::unlock();
    sound::pump();
    catalog.remaining_ms = block[2];
    if (int(stream[0]) < 1) {
        catalog.current_track = choose_track();
        if (catalog.current_track != -1) {
            publish_track(catalog.current_track, volume);
            std::strcpy(catalog.current_path, catalog.paths[catalog.current_track]);
            player->block = streams::enqueue_file(catalog.stream, 0, catalog.current_path, 0);
            sound::lock();
            runtime::race_audio::set_stream_volume(catalog.stream, volume);
            sound::unlock();
            return;
        }
    } else if (!configuration<Dword>(audio_layout::music_muted)) {
        if (!player->last_volume && volume)
            sound::field<Byte>(&catalog.notify, 0) = 1;
        player->last_volume = volume;
    }
}

void start_frontend_music(MusicPlayer *player) {
    if (!disable_music) {
        initialize_music_stream(player);
        library().selection = -1;
        begin_track(player, -1);
    }
}

void restore_music_volume() {
    if (configuration<Dword>(audio_layout::audio_enabled) && library().playing) {
        if (option<int>(audio_layout::music_source) != -3) {
            sound::lock();
            runtime::race_audio::set_stream_volume(
                library().stream,
                runtime::race_audio::group_volume(configuration<Dword>(audio_layout::music_volume), 5, 127));
            sound::unlock();
        } else
            runtime::race_audio::pending::set_cd_audio_volume(
                runtime::race_audio::group_volume(configuration<Dword>(audio_layout::music_volume), 5, 127));
    }
}

int has_multichannel_audio() {
    sound::backend::load_configuration(audio_settings);
    if (sound::field<Byte>(audio_settings, 10) > 15)
        for (unsigned format = 0; format < sound::field<Byte>(audio_settings, 19); ++format)
            if (sound::field<std::uint16_t>(audio_settings, 20 + format * 2) & 16)
                return 1;
    return 0;
}
} // namespace nfs::game::audio
