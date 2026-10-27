#include "race_audio_distance.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/audio/backend/sound_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::race_audio {

int pending::rounded_point_distance(const float *first, const float *second) {
    float difference[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        at<volatile float>(difference, axis * sizeof(float)) =
            float(static_cast<long double>(second[axis]) - first[axis]);
    const volatile float distance = float(hardware::vector_length(difference));
    return hardware::current_integer_f32(distance);
}

Dword pending::spatial_pan(const void *listener, const void *position) {
    float direction[3];
    for (unsigned i = 0; i < 3; ++i)
        at<volatile float>(direction, i * 4) =
            float(static_cast<long double>(at<float>(position, i * 4)) -
                  at<float>(listener, i * 4));
    const volatile float forward_sample =
        float(hardware::dot_product(direction, reinterpret_cast<const float *>(bytes(listener) + 36)) *
              65536.f);
    int forward = hardware::current_integer_f32(forward_sample);
    Dword *rules = static_cast<Dword *>(system(72));
    bool reverse = network::packets::rule_value(rules, 6) == 2;
    if (!reverse)
        reverse = network::packets::rule_value(rules, 6) == 3;
    auto side =
        hardware::dot_product(direction, reinterpret_cast<const float *>(bytes(listener) + 12));
    if (reverse)
        side = side * -1.0f;
    const volatile float lateral_sample = float(side * 65536.f);
    int lateral = hardware::current_integer_f32(lateral_sample);
    return (Dword(fixed_atan2(lateral >> 8, forward >> 8)) & 1023) << 6;
}
// Distance cutoff depends on the replay state.
void pending::submit_nearest(Dword channel, Dword event, Dword scale, Dword pitch,
                                      Dword squared_distance, Dword pan) {
    if (!network::graphics_enabled)
        return;
    const void *rules = system(72);
    const int cutoff = at<volatile Dword>(rules, 140) ? 10000 : 5625;
    int level = 0;
    if (static_cast<int>(squared_distance) < cutoff) {
        const Dword numerator_cutoff = at<volatile Dword>(rules, 140) ? 10000 : 5625;
        const int denominator = at<volatile Dword>(rules, 140) ? 10000 : 5625;
        level = static_cast<int>((numerator_cutoff - squared_distance) * 127) / denominator;
    }
    const long double scaled = static_cast<long double>(level) * 1.39999997615814208984375f;
    level = int(Dword(hardware::truncate_integer_native80(&scaled)));
    if (level < 0)
        level = 0;
    else if (level > 127)
        level = 127;
    if (!level && (channel & 255) < 112) {
        stop_event(channel);
        return;
    }
    play_event(channel, event, Dword(static_cast<int>(scale * pitch) / 65536), Dword(level),
                        pan, 0);
}

void pending::submit_distanced(Dword channel, Dword event, Dword scale, Dword pitch, Dword distance,
                               Dword pan, Dword radius, Dword maximum, Dword minimum, Dword flags) {
    if (!network::graphics_enabled)
        return;
    if (static_cast<int>(distance) < static_cast<int>(radius)) {
        const int attenuated =
            static_cast<int>((radius - distance) * (maximum - minimum)) / static_cast<int>(radius);
        int volume = int(Dword(attenuated) + minimum);
        if (volume >= 128)
            volume = 127;
        if (volume) {
            play_event(channel, event, Dword(static_cast<int>(scale * pitch) / 65536),
                                Dword(volume), pan, flags);
            return;
        }
    }
    if ((channel & 255) < 112)
        stop_event(channel);
}
} // namespace nfs::game::runtime::race_audio
