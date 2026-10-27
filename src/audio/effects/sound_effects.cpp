#include "nfs/audio/backend/sound_runtime.hpp"
namespace nfs::sound {

void submit_pan(int index) {
    Voice *voice = control.voices + index;
    BackendVoice *backend = backends + index;
    backend_call<void>(backend, 232, field<Dword>(backend, 172), field<std::int16_t>(voice, 24),
                       int(field<std::int16_t>(voice, 26)));
}

int submit_effect(int index, int effect) {
    BackendVoice *backend = backends + index;
    backend_call<void>(backend, 228, field<Dword>(backend, 180),
                       int(field<std::int16_t>(control.voices + index, 86 + effect * 2)));
    return 0;
}

void calculate_effect(int effect, int index) {
    Voice *voice = control.voices + index;
    void *group = effect_group(effect, field<std::uint16_t>(voice, 32));
    int value = field<std::int8_t>(voice, 85 + effect) *
                field<std::int8_t>(voice, 84 + effect) * field<std::int8_t>(group, 2);
    field<std::int16_t>(voice, 86 + effect * 2) = static_cast<std::int16_t>(value >> 6);
}

int set_voice_pan(Dword handle, std::int16_t pan, int spread) {
    if (!control.initialized)
        return -10;
    int clamped = spread;
    if (spread < 16384) {
        if (spread < -16384)
            clamped = -16384;
    } else
        clamped = 16383;
    const int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor)) {
            Voice *voice = control.voices + cursor;
            field<std::int16_t>(voice, 26) = static_cast<std::int16_t>(clamped);
            field<std::int16_t>(voice, 24) =
                static_cast<std::int16_t>(field<std::int16_t>(voice, 70) + pan);
            submit_pan(cursor);
        }
    }
    return first;
}

int set_voice_effect(Dword handle, int effect, Byte value) {
    if (!control.initialized)
        return -10;
    const int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor)) {
            field<Byte>(control.voices + cursor, 85 + effect) = value;
            calculate_effect(effect, cursor);
            submit_effect(cursor, effect);
        }
    }
    return first;
}

int voice_time_remaining(Dword handle) {
    int maximum = -7;
    if (!control.initialized)
        return -10;
    const int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor)) {
            Voice *voice = control.voices + cursor;
            if (field<int>(voice, 20) < 1) {
                const int position = voice_position(cursor);
                if (position >= 0) {
                    int remaining;
                    if (field<std::uint16_t>(voice, 126) < 17)
                        remaining = 2147483647;
                    else {
                        remaining =
                            static_cast<int>((field<Dword>(voice, 16) - Dword(position)) * 100) /
                            int(field<std::uint16_t>(voice, 28));
                        remaining = static_cast<int>(Dword(remaining) << 12) /
                                    int(field<std::uint16_t>(voice, 126));
                        remaining = static_cast<int>(Dword(remaining) << 12) /
                                    int(field<std::uint16_t>(voice, 94));
                    }
                    if (maximum < remaining)
                        maximum = remaining;
                }
            }
        }
    }
    return maximum;
}

int voice_position(int index) {
    BackendVoice *backend = backends + index;
    return backend_call<int>(backend, 252, field<Dword>(backend, 200));
}
} // namespace nfs::sound
