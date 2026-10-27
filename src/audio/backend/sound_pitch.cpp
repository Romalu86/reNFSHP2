#include "nfs/audio/backend/sound_runtime.hpp"
namespace nfs::sound {

int pitch_multiplier(int cents) {
    Dword multiplier = 4096;
    if (cents > 1199) {
        unsigned octaves = Dword(cents) / 1200;
        cents %= 1200;
        do {
            multiplier *= 2;
        } while (--octaves);
    }
    if (cents < -1199) {
        int octaves = static_cast<int>((Dword(0) - Dword(cents) - 1200) / 1200) + 1;
        cents = static_cast<int>(Dword(cents) + Dword(octaves) * 1200);
        do {
            multiplier = Dword(static_cast<int>(multiplier) >> 1);
        } while (--octaves);
    }
    int index = static_cast<int>(Dword(cents) * 13981) >> 16;
    if (index < -255)
        index = -255;
    else if (index >= 0)
        return static_cast<int>((Dword(pitch_curve[index]) + 256) * multiplier) >> 8;
    return static_cast<int>((Dword(pitch_curve[256 + index]) + 256) * multiplier) >> 9;
}

void calculate_pitch(int index) {
    Voice *voice = control.voices + index;
    if (field<std::int16_t>(voice, 122) == 0) {
        int cents = field<std::int16_t>(voice, 120);
        if (field<std::int16_t>(voice, 118) != 0) {
            const auto *table = field<std::int8_t *>(voice, 104);
            const int position = field<std::int8_t>(voice, 88);
            const int value = table ? table[position] : position;
            cents += ((value - 64) * field<std::int16_t>(voice, 118)) >> 6;
        }
        const auto *modulation = field<std::int8_t *>(voice, 112);
        if (modulation)
            cents +=
                ((modulation[field<Byte>(voice, 92)] - 64) * field<std::int16_t>(voice, 116)) >>
                6;
        field<std::uint16_t>(voice, 122) = static_cast<std::uint16_t>(pitch_multiplier(cents));
    }
    const Dword product =
        Dword(field<std::uint16_t>(voice, 124)) * field<std::uint16_t>(voice, 122);
    field<std::uint16_t>(voice, 126) = static_cast<std::uint16_t>(static_cast<int>(product) >> 12);
}

int submit_pitch(int index) {
    Voice *voice = control.voices + index;
    auto rate = field<std::uint16_t>(voice, 126);
    if ((field<Byte>(voice, 32) & 2) && rate > 8192)
        rate = 8192;
    BackendVoice *backend = backends + index;
    backend_call<void>(backend, 220, field<Dword>(backend, 184), rate);
    return 0;
}

int set_voice_rate(Dword handle, Dword rate) {
    if (!control.initialized)
        return -10;
    const int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor)) {
            Voice *voice = control.voices + cursor;
            if (Dword(field<std::uint16_t>(voice, 124)) == rate)
                return 0;
            field<std::uint16_t>(voice, 124) = static_cast<std::uint16_t>(rate);
            calculate_pitch(cursor);
            submit_pitch(cursor);
        }
    }
    return first;
}

int set_voice_pitch(Dword handle, int position) {
    if (!control.initialized)
        return -10;
    const int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor)) {
            Voice *voice = control.voices + cursor;
            if (field<std::int8_t>(voice, 88) == position ||
                field<std::int16_t>(voice, 118) == 0)
                return 0;
            field<std::int8_t>(voice, 88) = static_cast<std::int8_t>(position);
            field<std::uint16_t>(voice, 122) = 0;
            calculate_pitch(cursor);
            submit_pitch(cursor);
        }
    }
    return first;
}
} // namespace nfs::sound
