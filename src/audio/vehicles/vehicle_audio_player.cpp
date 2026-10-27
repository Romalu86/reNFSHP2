#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio::vehicle {
using namespace state;
namespace ra = runtime::race_audio;
static void *race_progress(void *car) {
    auto *object = at<Byte *>(car, 7988);
    auto *base = at<Byte *>(object, 4);
    int offset = at<int>(base, 8);
    return runtime::invoke<void *>(object + 4 + offset, 2);
}

void update_player_events(void *car, int listener) {
    ra::debug_sound(0, 1, "EngVol: %d", configuration<int>(audio_layout::player_volume));
    ra::debug_sound(3, 1, "GasLevel: %d", at<Byte>(car, 1583));
    Dword flags = at<Dword>(car, 1792);
    if (flags & 4096) {
        unsigned baseline_offset = listener ? audio_layout::baseline_second_player_volume : audio_layout::baseline_player_volume;
        const long double scaled =
            static_cast<long double>(configuration<int>(baseline_offset)) * 0.4000000059604644775390625f;
        int target = int(Dword(runtime::hardware::truncate_integer_native80(&scaled)));
        set_volume_fade(listener != 0, target, 5.0f);
        set_volume_fade(listener != 0, -2, 5.0f);
        at<Dword>(car, 1792) = flags & 4294963199u;
    }
    void *progress = race_progress(car);
    if (progress && at<int>(runtime::system(72), 176) != 2) {
        int lap = at<int>(progress, 568);
        if (option<int>(audio_layout::last_announced_lap) < lap) {
            option<int>(audio_layout::last_announced_lap) = lap;
            if (lap == at<int>(runtime::system(72), 228) - 1)
                request_lap_announcement(runtime::system(84), 0);
        }
    }
}

void update_player_engine(void *car, int distance, int pitch, Dword pan, int direction,
                          int listener) {
    int volume = configuration<int>(listener ? audio_layout::second_player_volume : audio_layout::player_volume);
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    if (configuration<int>(audio_layout::player_volume) < 0)
        volume = configuration<int>(audio_layout::player_volume) = configuration<int>(audio_layout::baseline_player_volume);
    if (configuration<int>(audio_layout::second_player_volume) < 0)
        volume = configuration<int>(audio_layout::second_player_volume) = configuration<int>(audio_layout::baseline_second_player_volume);
    update_player_events(car, listener);
    ra::pending::update_finished_car(car, listener);
    int attenuation =
        at<Dword>(runtime::system(72), 140) ? attenuate(distance, cutoff(), 127) : 127;
    configuration<int>(1704) = configuration<int>(1696);
    configuration<int>(1700) = configuration<int>(1692);
    if (at<int>(runtime::system(72), 176) == 1) {
        apply_reverb_preset(at<Byte>(at<void *>(car, 1768), 584));
        configuration<int>(1684) = 1;
    }
    Dword index = at<Dword>(car, 1772);
    Byte *descriptor = index < 24 ? bytes(runtime::system(72)) + 248 + index * 84 : nullptr;
    int &gas = at<int>(effects_storage, 44);
    if (!at<int>(descriptor, 76)) {
        if (at<Byte>(car, 1576))
            gas = int(Dword(gas) + 16);
        else
            gas = 0;
    } else
        gas = at<Byte>(car, 1576);
    gas = clamp_value(gas, 0, 255);
    int &smoothed = option<int>(audio_layout::player_engine_level + index * 4);
    int difference = int(Dword(gas) - Dword(smoothed));
    smoothed =
        clamp_value(int(Dword(smoothed) + Dword(difference / (gas < smoothed ? 8 : 2))), 0, 255);
    int rpm = normalized_rpm(car);
    int gain = configuration<Dword>(12) ? 0 : ra::group_volume(volume, 1, attenuation);
    set_engine_parameters(gain, std::uint16_t((wrapped_product(rpm, 408) >> 7) + 32), smoothed >> 1,
                          distance, pitch, std::uint16_t(pan), direction, game::tick_services[0],
                          listener);
}

void request_lap_announcement(void *speech, int event) {
    if (!at<Dword>(speech, 20) || at<Dword>(runtime::system(72), 140))
        return;
    if (event == 1) {
        Dword message = pending::create_speech_event(0, 0, 160);
        pending::speech_sink(message, 1, 1);
        pending::flush_speech_events(0);
    } else if (!event) {
        Dword message = pending::create_speech_event(0, 0, 160);
        pending::speech_sink(message, 1, 1048576);
        pending::flush_speech_events(0);
    } else if (event == 2) {
        Dword message = pending::create_speech_event(0, 0, 220);
        pending::speech_sink(message, 0);
        pending::flush_speech_events(0);
    }
}
} // namespace nfs::game::audio::vehicle

namespace nfs::game::runtime::race_audio::pending {

void update_finished_car(void *car, Dword listener) {
    audio::vehicle::update_wind(car, listener);
    if (at<int>(system(72), 176) != 2 || listener != 1)
        audio::vehicle::update_track_emitters(car);
}
} // namespace nfs::game::runtime::race_audio::pending
