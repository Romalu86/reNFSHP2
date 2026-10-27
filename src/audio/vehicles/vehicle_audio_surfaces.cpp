#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio::vehicle {
using namespace runtime::race_audio;

void select_surface_sound(Dword channel, int event_kind, Dword surface, int speed, int distance,
                          Dword pan) {
    if (!state::configuration<Dword>(audio_layout::audio_enabled))
        return;
    Dword id = channel & 255;
    int volume = attenuate(int(Dword(distance) * 2), cutoff(), 127);
    if (id == 122 || id > 111)
        return;
    if (speed < 1) {
        stop_event(channel);
        return;
    }
    int event = 42;
    if (event_kind == 18) {
        if (!Byte(surface >> 8)) {
            switch (surface) {
            case 1:
            case 27:
                event = 43;
                break;
            case 23:
            case 25:
                event = 41;
                break;
            case 28:
                event = 44;
                break;
            case 29:
                event = 45;
                break;
            }
        } else
            switch (surface & 255) {
            case 23:
            case 25:
                event = 46;
                break;
            case 29:
                event = 109;
                break;
            case 30:
                event = 39;
                break;
            }
    } else if (event_kind == 15)
        event = surface == 28 ? 44 : 40;
    else if (!event_kind)
        event = (surface == 28 || surface == 25) ? 46 : 48;
    int speed_pitch = wrapped_product(speed, 127) / 655360;
    if (speed_pitch > 126)
        speed_pitch = 127;
    unsigned slot = ((channel >> 8) & 255) * 124 + id;
    int pitch = speed_pitch;
    if (id != 3) {
        if (state::channels()[slot].event == Dword(event))
            pitch = state::channel_variation[slot];
        else
            state::channel_variation[slot] = Byte(pitch);
    }
    if (id == 3 || id == 4 || id == 7 || event == 47 || event == 46 || event == 39 ||
        (id > 53 && id < 58)) {
        volume = wrapped_product(volume, speed_pitch) / 127;
        if ((id == 3 || id == 4) && event == 42) {
            volume = int(Dword(volume) * 2);
            if (volume > 127)
                volume = 127;
            runtime::race_audio::pending::play_event(channel, event, pitch, volume, pan,
                                                              0);
            return;
        }
        if (event == 46 || event == 39)
            pitch = volume > 64 ? 64 : volume;
    }
    if (event == 44) {
        pitch = int(Dword(pitch) - 60);
        if (pitch < 0)
            pitch = 0;
    }
    runtime::race_audio::pending::play_event(channel, event, pitch, volume, pan, 0);
}

void submit_distant_engine(int distance, int pitch, Dword pan, int listener) {
    if (state::configuration<Dword>(audio_layout::audio_enabled))
        runtime::race_audio::pending::play_event(
            (listener ? 256 : 0) | 1, 0, pitch, attenuate(distance, cutoff(), 127), pan, 1);
}

void stop_distant_engine() {
    if (state::configuration<Dword>(audio_layout::audio_enabled) && state::channels()[1].voice != Dword(-1)) {
        sound::lock();
        sound::fade_voice_volume(state::channels()[1].voice, 5, -1);
        state::channels()[1].voice = state::channels()[1].event = Dword(-1);
        sound::unlock();
    }
}

void submit_wheel_sound(Dword channel, Dword event, int scale, int pitch, int distance, Dword pan,
                        int contact) {
    if (!state::configuration<Dword>(audio_layout::audio_enabled))
        return;
    Dword id = channel & 255;
    int volume;
    if (id >= 14 && id <= 17) {
        volume = attenuate(distance, cutoff() >> 3, 127);
        state::world<int>(6916) = contact > 40 ? 40 : contact;
        volume = wrapped_product(state::world<int>(6916), volume) / 40;
    } else {
        int &gain = at<int>(state::effects_storage, 48);
        int target = int(127 - Dword(state::option<int>(audio_layout::player_attenuation_steps)) * 10);
        if (target < gain)
            --gain;
        else if (gain < target)
            ++gain;
        volume = attenuate(distance, cutoff(), gain);
    }
    if (volume > 127)
        volume = 127;
    if (!volume && id < 112) {
        stop_event(channel);
        return;
    }
    runtime::race_audio::pending::play_event(channel, event, fixed_pitch(scale, pitch),
                                                      volume, pan, 0);
}

void submit_tyre_sound(Dword channel, Dword event, int scale, int pitch, int distance, Dword pan,
                       int direction) {
    if (!state::configuration<Dword>(audio_layout::audio_enabled))
        return;
    Dword id = channel & 255;
    int &gain = at<int>(state::effects_storage, 52);
    int target = int(127 - Dword(state::option<int>(audio_layout::player_attenuation_steps)) * 10);
    if (target < gain)
        --gain;
    else if (gain < target)
        ++gain;
    int range = cutoff();
    if ((direction < 0 && id > 37 && id < 54) || (direction >= 0 && id >= 22 && id <= 37))
        range >>= 4;
    int volume = attenuate(distance, range, gain);
    if (!scale)
        volume = 0;
    if (volume > 127)
        volume = 127;
    if (!volume && id < 112) {
        stop_event(channel);
        return;
    }
    runtime::race_audio::pending::play_event(channel, event, fixed_pitch(scale, pitch),
                                                      volume, pan, 0);
}

void submit_surface_sound(Dword channel, int event, Dword surface, int speed, int distance,
                          Dword pan) {
    if (!state::configuration<Dword>(audio_layout::audio_enabled))
        return;
    int range = cutoff();
    if ((distance < range && attenuate(distance, range, 127)) || (channel & 255) > 111)
        select_surface_sound(channel, event, surface, speed, distance, pan);
    else
        stop_event(channel);
}
} // namespace nfs::game::audio::vehicle
