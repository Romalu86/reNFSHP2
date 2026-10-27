#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <mmsystem.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;
// The finite retry loop and its lack of a sleep are saved behavior.
int wait_for_cd() {
    if (!option<Dword>(audio_layout::skip_cd_ready_wait)) {
        char status[128];
        int ready = 0, attempt = 0;
        do {
            mciSendStringA("status cdaudio ready", status, 128, nullptr);
            if (!std::strncmp("true", status, 5))
                ready = 1;
            if (++attempt > 500)
                return 0;
        } while (!ready);
        if (attempt > 500)
            return 0;
    }
    return 1;
}

void read_cd_tracks() {
    char status[128];
    mciSendStringA("status cdaudio number of tracks", status, 128, nullptr);
    option<int>(audio_layout::cd_track_count) = std::atoi(status);
}

int next_cd_track() {
    int count = option<int>(audio_layout::cd_track_count);
    read_cd_tracks();
    if (count != option<int>(audio_layout::cd_track_count)) {
        option<int>(audio_layout::cd_current_track) = 1;
        return 1;
    }
    ++option<Dword>(audio_layout::cd_current_track);
    if (option<int>(audio_layout::cd_track_count) < option<int>(audio_layout::cd_current_track)) {
        option<int>(audio_layout::cd_current_track) = 1;
        return 1;
    }
    return option<int>(audio_layout::cd_current_track);
}

void play_cd_track(int track) {
    if (library().selection != -3)
        return;
    if (!option<Dword>(audio_layout::cd_open)) {
        mciSendStringA("open cdaudio", nullptr, 0, nullptr);
        option<Dword>(audio_layout::cd_open) = 1;
    }
    if (library().playing != 1) {
        char status[128];
        mciSendStringA("status cdaudio number of tracks", status, 128, nullptr);
        if (std::atoi(status) > 1) {
            library().playing = 1;
            read_cd_tracks();
            if (option<int>(audio_layout::cd_track_count)) {
                mciSendStringA("set cdaudio time format tmsf", nullptr, 0, nullptr);
                char command[256];
                std::sprintf(command, "play cdaudio from %d", track);
                mciSendStringA(command, nullptr, 0, nullptr);
                if (!configuration<Dword>(audio_layout::music_muted) && configuration<Dword>(audio_layout::audio_enabled) &&
                    library().playing)
                    restore_music_volume();
            }
        }
    }
}

void stop_cd_track() {
    char status[128];
    library().playing = 0;
    mciSendStringA("status cdaudio current track", status, 128, nullptr);
    option<int>(audio_layout::cd_current_track) = std::atoi(status);
    mciSendStringA("stop cdaudio", nullptr, 0, nullptr);
}

void update_cd_music() {
    const long long milliseconds = GetTickCount();
    const volatile float elapsed =
        float(static_cast<long double>(milliseconds) * 60.0f * 0.001f);
    const int tick = runtime::hardware::current_integer_f32(elapsed);
    extern void update_speech_stream(void *);
    update_speech_stream(lifecycle::manager->slots[21]);
    if (sound::field<Byte>(lifecycle::manager->slots[13], 81296)) {
        int volume = runtime::race_audio::group_volume(configuration<Dword>(audio_layout::music_volume), 4, 127);
        if (!option<Dword>(audio_layout::speech_stream_active)) {
            if (option<Dword>(audio_layout::music_ducked)) {
                runtime::race_audio::pending::set_cd_audio_volume(volume);
                option<Dword>(audio_layout::music_ducked) = 0;
            }
        } else if (!option<Dword>(audio_layout::music_ducked)) {
            runtime::race_audio::pending::set_cd_audio_volume(volume >> 2);
            option<Dword>(audio_layout::music_ducked) = 1;
        }
    }
    extern int cd_poll_tick;
    if (!option<Dword>(audio_layout::suppress_cd_poll) && int(Dword(cd_poll_tick) + 128) <= tick) {
        cd_poll_tick = tick;
        char status[128];
        mciSendStringA("status cdaudio ready", status, 128, nullptr);
        if (!std::strncmp("true", status, 5)) {
            mciSendStringA("status cdaudio mode", status, 128, nullptr);
            if (!std::strncmp(status, "stopped", 8)) {
                library().playing = 0;
                int track = next_cd_track();
                play_cd_track(track);
            }
        }
    }
}
int cd_poll_tick{};

void poll_music(Dword) {
    if (library().selection == -3)
        update_cd_music();
    else
        update_music(reinterpret_cast<MusicPlayer *>(lifecycle::manager->slots[20]),
                     library().selection);
}
} // namespace nfs::game::audio
