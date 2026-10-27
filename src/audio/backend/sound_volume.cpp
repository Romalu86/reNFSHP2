#include "nfs/audio/backend/sound_runtime.hpp"
namespace nfs::sound {
namespace {
int multiply(int left, int right) {
    return static_cast<int>(Dword(left) * Dword(right));
}
} // namespace

void calculate_volume(int index) {
    Voice *voice = control.voices + index;
    int volume = multiply(field<std::int8_t>(voice, 68), field<std::int16_t>(voice, 62));
    volume = multiply(volume, field<std::int16_t>(voice, 54));
    volume = multiply(volume, control.master_volume);
    field<std::int8_t>(voice, 69) = static_cast<std::int8_t>(volume / 2048383);
    auto *modulation = field<std::int8_t *>(voice, 108);
    if (modulation) {
        volume = modulation[field<Byte>(voice, 91)] * field<std::int8_t>(voice, 69);
        field<std::int8_t>(voice, 69) = static_cast<std::int8_t>(volume / 127);
    }
    auto *remap = field<Byte *>(voice, 100);
    if (remap)
        field<Byte>(voice, 69) = remap[field<std::int8_t>(voice, 69)];
}

void submit_volume(int index) {
    Voice *voice = control.voices + index;
    if (!(field<Byte>(voice, 32) & 4)) {
        auto *owner = field<Byte *>(backends + index, 176);
        auto *buffer = field<Dword *>(owner, 60);
        using SetVolume = int(__stdcall *)(Dword *, int);
        const int volume = volume_lookup_storage[128 + field<std::int8_t>(voice, 69)];
        reinterpret_cast<SetVolume *>(buffer[0])[60 / 4](buffer, volume);
    } else {
        const float scale = static_cast<float>(field<std::int8_t>(voice, 83)) *
                            static_cast<float>(field<std::int8_t>(voice, 69)) *
                            0.00000000189215132451181489159353077411651611328125f;
        for (int channel = 0; channel < field<Byte>(voice, 31); ++channel) {
            const int mix = field<std::int16_t>(voice, 4 + channel * 2);
            BackendVoice *backend = backends + mix;
            for (int output = 0; output < static_cast<int>(output_channels & 255); ++output)
                set_mix_volume(mix, output,
                               static_cast<float>(field<std::int16_t>(backend, output * 2)) *
                                   scale);
        }
    }
}

void set_mix_volume(int index, int channel, float value) {
    field<float>(mix_voices + index, 20 + channel * 4) = value;
    field<Byte>(mix_voices + index, 1) = 1;
}

Dword set_master_volume(Byte value) {
    if (!control.initialized)
        return Dword(-10);
    lock();
    control.master_volume = static_cast<std::int8_t>(value);
    for (int index = 0; index < control.voice_count; ++index) {
        Voice *voice = control.voices + index;
        if (field<Byte>(voice, 93) == 1 && field<int>(voice, 0) >= 0) {
            calculate_volume(index);
            submit_volume(index);
        }
    }
    unlock();
    return 0;
}

int set_voice_volume(Dword handle, int value) {
    if (!control.initialized)
        return -10;
    const int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        const Dword fixed = Dword(value) << 16;
        while (next_voice(first, &cursor)) {
            Voice *voice = control.voices + cursor;
            field<Dword>(voice, 44) = 0;
            if (field<Dword>(voice, 52) == fixed)
                return 0;
            field<Dword>(voice, 52) = fixed;
            calculate_volume(cursor);
            submit_volume(cursor);
            if (field<std::int16_t>(voice, 86) > 0)
                submit_effect(field<std::int16_t>(voice, 4), 0);
        }
    }
    return first;
}

int fade_voice_volume(Dword handle, int steps, int value) {
    if (!control.initialized)
        return -10;
    const int first = find_voice(handle);
    if (first >= 0) {
        if (steps < 1)
            steps = 1;
        int cursor = -1;
        while (next_voice(first, &cursor)) {
            Voice *voice = control.voices + cursor;
            const Dword target = Dword(value) << 16;
            field<Dword>(voice, 48) = target;
            field<int>(voice, 44) = static_cast<int>(target - field<Dword>(voice, 52)) / steps;
        }
    }
    return first;
}

int voice_missing(Dword handle) {
    if (!control.initialized)
        return -10;
    return find_voice(handle) < 0;
}

int voice_volume(Dword handle) {
    if (!control.initialized)
        return -10;
    const int index = find_voice(handle);
    if (index < 0)
        return -8;
    return field<std::int16_t>(control.voices + index, 54);
}
} // namespace nfs::sound
