#include "nfs/audio/backend/sound_ownership.hpp"
#include <climits>
namespace nfs::sound::backend {
// Single RET, called before producer callbacks.
void voice_tick_boundary() {
}

void update_audio_voices() {
    ++control.unknown_2;
    ++control.unknown_c;
    voice_tick_boundary();
    for (int i = 0; i < control.producer_count; ++i)
        control.producers[i]();
    for (int i = 0; i < control.voice_count; ++i) {
        auto *voice = control.voices + i;
        if (field<Byte>(voice, 93) != 1 || field<int>(voice, 0) < 0)
            continue;
        if (field<Dword>(voice, 112)) {
            auto &cursor = field<Byte>(voice, 92);
            ++cursor;
            if (cursor >= field<Byte>(voice, 90))
                cursor = 0;
            field<std::uint16_t>(voice, 122) = 0;
            calculate_pitch(i);
            submit_pitch(i);
        }
        bool dirty = false;
        if (field<Dword>(voice, 108)) {
            auto &cursor = field<Byte>(voice, 91);
            ++cursor;
            dirty = true;
            if (cursor >= field<Byte>(voice, 89))
                cursor = 0;
        }
        int increment = field<int>(voice, 44);
        if (increment) {
            dirty = true;
            auto &value = field<int>(voice, 52);
            value = int(Dword(value) + Dword(increment));
            int target = field<int>(voice, 48);
            if ((increment < 0 && value <= target) || (increment >= 0 && target <= value)) {
                value = target;
                field<Dword>(voice, 44) = 0;
            }
            if (value < 0) {
                stop(field<Dword>(voice, 0));
                continue;
            }
        }
        auto &remaining = field<int>(voice, 64);
        remaining = int(Dword(remaining) - 1);
        if (field<Dword>(voice, 56)) {
            dirty = true;
            field<Dword>(voice, 60) += field<Dword>(voice, 56);
        }
        if (!remaining) {
            auto &segment = field<std::int8_t>(voice, 81);
            segment = std::int8_t(segment + 1);
            if (segment >= field<std::int8_t>(voice, 80)) {
                stop(field<Dword>(voice, 0));
                continue;
            }
            const auto *curve = field<const int *>(voice, 96) + segment * 2;
            remaining = curve[0];
            if (remaining < 0)
                remaining = INT_MAX;
            field<int>(voice, 56) =
                int((Dword(curve[1]) << 16) - field<Dword>(voice, 60)) / remaining;
        }
        if (dirty) {
            calculate_volume(i);
            if (field<int>(voice, 0) >= 0)
                submit_volume(i);
        }
    }
    --control.unknown_2;
}
} // namespace nfs::sound::backend
