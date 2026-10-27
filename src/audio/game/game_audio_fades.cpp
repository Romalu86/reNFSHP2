#include "nfs/audio/music/music_clients.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;
struct VolumeFade {
    float baseline, current, target, rate;
    int last_tick, deadline;
    Dword reserved;
};
static_assert(sizeof(VolumeFade) == 28);
static VolumeFade *fades() {
    return &world<VolumeFade>(audio_layout::volume_fades);
}

void initialize_volume_fades() {
    const unsigned source_offsets[12] = {audio_layout::baseline_player_volume, audio_layout::baseline_second_player_volume, audio_layout::baseline_opponent_volume, audio_layout::baseline_player_skid_volume, audio_layout::baseline_boom_collision_volume, audio_layout::baseline_boom_loop_volume,
                                  audio_layout::baseline_car_collision_volume, audio_layout::baseline_track_volume, audio_layout::baseline_traffic_volume, audio_layout::baseline_speech_volume, audio_layout::baseline_music_volume, audio_layout::baseline_boom_oneshot_volume};
    const unsigned initialization_order[12] = {0, 1, 2, 3, 4, 5, 11, 6, 7, 8, 9, 10};
    for (unsigned index : initialization_order)
        fades()[index].baseline = static_cast<float>(configuration<int>(source_offsets[index]));
    for (int index = 0; index < 12; ++index) {
        VolumeFade &fade = fades()[index];
        const Dword baseline = sound::field<volatile Dword>(&fade, 0);
        sound::field<volatile Dword>(&fade, 4) = baseline;
        sound::field<volatile Dword>(&fade, 8) = baseline;
        sound::field<volatile Dword>(&fade, 12) = 0;
        sound::field<volatile Dword>(&fade, 16) = 0;
        set_volume_fade(index, -1, 0);
    }
}

void set_volume_fade(int group, int target, float duration) {
    VolumeFade &fade = fades()[group];
    if (target == -1)
        sound::field<volatile Dword>(&fade, 8) = sound::field<volatile Dword>(&fade, 0);
    else if (target == -2) {
        const long double retained_duration = static_cast<long double>(duration);
        world<Dword>(audio_layout::volume_fade_delay) =
            Dword(runtime::hardware::truncate_integer_native80(&retained_duration));
    } else
        fade.target = static_cast<float>(target);
    fade.last_tick = int(graphics::unresolved::camera_state);
    long double value = static_cast<long double>(sound::field<volatile float>(&fade, 8));
    // The saved unordered comparisons leave a NaN target unchanged.
    if (value < 0)
        value = 0;
    else if (value > 127)
        value = 127;
    sound::field<volatile float>(&fade, 8) = float(value);
    if (duration > 0) {
        fade.rate = float((value - fade.current) / static_cast<long double>(duration));
        return;
    }
    sound::field<volatile Dword>(&fade, 12) = 0;
    update_volume_fades();
}
// This client updates volume envelopes; it is not an announcer dispatcher.
void update_volume_fades() {
    const unsigned destination_offsets[12] = {audio_layout::player_volume, audio_layout::second_player_volume, audio_layout::opponent_volume, audio_layout::player_skid_volume, audio_layout::boom_collision_volume, audio_layout::boom_loop_volume,
                                       audio_layout::car_collision_volume, audio_layout::track_volume, audio_layout::traffic_volume, audio_layout::speech_volume, audio_layout::music_volume, audio_layout::boom_oneshot_volume};
    int tick = int(graphics::unresolved::camera_state);
    for (int group = 0; group < 12; ++group) {
        VolumeFade &fade = fades()[group];
        if (tick < fade.last_tick)
            fade.last_tick = tick;
        if (fade.deadline && fade.deadline < tick) {
            set_volume_fade(group, -1, 15);
            tick = int(graphics::unresolved::camera_state);
        }
        if (fade.current != fade.target) {
            int elapsed = int(Dword(tick) - Dword(fade.last_tick));
            fade.last_tick = tick;
            fade.current = float(static_cast<long double>(elapsed) * fade.rate + fade.current);
            if (fade.rate == 0 || (fade.rate < 0 && fade.current < fade.target) ||
                (fade.rate > 0 && fade.target < fade.current))
                sound::field<volatile Dword>(&fade, 4) = sound::field<volatile Dword>(&fade, 8);
            const long double current = static_cast<long double>(sound::field<volatile float>(&fade, 4));
            configuration<Dword>(destination_offsets[group]) =
                Dword(runtime::hardware::truncate_integer_native80(&current));
            if (group == 10)
                restore_music_volume();
            apply_event_volumes();
            tick = int(graphics::unresolved::camera_state);
            fade.deadline = 0;
        } else if (fade.target != fade.baseline && !fade.deadline && world<Dword>(audio_layout::volume_fade_delay)) {
            fade.deadline = int(world<Dword>(audio_layout::volume_fade_delay) + Dword(tick));
        }
    }
}
} // namespace nfs::game::audio
