#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::sound::pending {
extern Byte &backend_count, &reserved_voice_count;
extern std::uint16_t *const backend_formats;
int allocate_layers(Dword, Dword, int *, int, int);
int start_backend(int, Dword, Dword, Dword, Dword, Dword, Dword, const Dword *, Dword,
                           const Dword *, Dword);

std::uint16_t pan_from_key(int key) { return pan_lookup[key]; }
// Other flag combinations leave the caller's limits untouched.
void voice_range(Byte format, int *first, int *end) {
    if (format & 4) {
        *first = 0;
        *end = reserved_voice_count;
    } else if (format & 16) {
        *first = reserved_voice_count;
        *end = static_cast<std::int16_t>(control.voice_count);
    }
}
// Match the format groups and channel restriction.
std::uint16_t next_layer_format(int *cursor, void *layer) {
    if (*cursor >= backend_count)
        return 0;
    do {
        std::uint16_t format = backend_formats[*cursor],
                      requested = field<std::uint16_t>(layer, 24), common = format & 1820;
        if (!(requested & 1820) || (common &= requested) != 0) {
            std::uint16_t group = format & 224;
            if (!(requested & 224) || (group &= requested) != 0) {
                common |= group;
                if (format & 16) {
                    if (!(requested & 3))
                        common |= format & 3;
                    else {
                        std::uint16_t bits = requested & format & 3;
                        common |= bits;
                        if (!bits)
                            common = 0;
                    }
                }
                if (field<std::int8_t>(layer, 3) != 1 && static_cast<std::int8_t>(common) < 0)
                    common = 0;
                if (common) {
                    ++*cursor;
                    return common;
                }
            }
        }
        ++*cursor;
    } while (*cursor < backend_count);
    return 0;
}
// Two parameters are saved ABI arguments but are not read.
int start_layer(void *layer, void *bank, int voice, Dword, Dword, Dword mode, Dword flags) {
    Dword sources[4]{};
    const int channels = field<std::int8_t>(layer, 3);
    for (int i = 0; i < channels; ++i)
        sources[i] = field<Dword>(layer, 104 + i * 4) + reinterpret_cast<Dword>(bank);
    return start_backend(voice, field<Dword>(layer, 92), field<Dword>(layer, 96), 0, 0,
                                  mode, flags, sources, field<Dword>(layer, 120),
                                  reinterpret_cast<Dword *>(static_cast<Byte *>(layer) + 124),
                                  field<Dword>(layer, 156));
}
// Create/retry layers with the saved per-voice field layout.
int create_layer(std::uint16_t bank_id, std::uint16_t event, void *bank,
                          const VoiceParameters *parameters, void *layer, int key, int velocity,
                          int variation) {
    const int volume = field<std::int8_t>(parameters, 0), pitch = field<std::int8_t>(parameters, 1),
              effect = field<std::int8_t>(parameters, 5);
    int format_cursor = 0, first, end;
    int result;
    for (;;) {
        result = -9;
        int format, index;
        do {
            format = next_layer_format(&format_cursor, layer);
            if (!format)
                return result;
            voice_range(Byte(format), &first, &end);
            index =
                allocate_layers(Dword(field<std::int8_t>(layer, 3)),
                                         Dword(field<std::int8_t>(layer, 12)), &result, first, end);
        } while (index < 0);
        Voice *voice = control.voices + index;
        int pan = field<std::int8_t>(layer, 17);
        if (field<std::int8_t>(layer, 18)) {
            pan += random_variation(field<std::int8_t>(layer, 18));
            if (pan < 0)
                pan = 0;
            else if (pan > 127)
                pan = 127;
        }
        field<std::uint16_t>(voice, 70) = pan_from_key(pan);
        std::uint16_t cents = field<std::uint16_t>(layer, 4);
        field<std::uint16_t>(voice, 120) = cents;
        cents = std::uint16_t(cents + std::uint16_t((key - field<std::int8_t>(layer, 13)) * 100));
        field<std::uint16_t>(voice, 120) = cents;
        if (variation) {
            cents = std::uint16_t(cents + variation);
            field<std::uint16_t>(voice, 120) = cents;
        }
        if (field<std::int16_t>(layer, 6))
            field<std::uint16_t>(voice, 120) =
                std::uint16_t(field<std::uint16_t>(voice, 120) +
                              random_variation(field<std::int16_t>(layer, 6)));
        field<Dword>(voice, 100) = field<Dword>(layer, 32);
        field<Dword>(voice, 104) = field<Dword>(layer, 36);
        field<Dword>(voice, 44) = 0;
        field<Dword>(voice, 52) = Dword(volume) << 16;
        int key_volume = field<std::int8_t>(layer, 20);
        if (field<std::int8_t>(layer, 21))
            key_volume += random_variation(field<std::int8_t>(layer, 21));
        if (key_volume >= 128)
            key_volume = 127;
        else if (key_volume < 0)
            key_volume = 0;
        field<Byte>(voice, 68) = Byte((key_volume * velocity) / 127);
        field<Byte>(voice, 88) = Byte(pitch);
        field<std::uint16_t>(voice, 118) = std::uint16_t(field<std::int8_t>(layer, 16) * 100);
        field<Dword>(voice, 96) = field<Dword>(layer, 40);
        field<Dword>(voice, 60) = Dword(field<std::int8_t>(layer, 26)) << 16;
        field<Byte>(voice, 81) = 0;
        field<Byte>(voice, 80) = field<Byte>(layer, 15);
        field<Byte>(voice, 82) = field<Byte>(layer, 14);
        const Dword *envelope = field<Dword *>(voice, 96);
        int duration = static_cast<int>(envelope[0]);
        if (duration < 0)
            duration = 2147483647;
        field<int>(voice, 64) = duration;
        field<int>(voice, 56) =
            static_cast<int>((envelope[1] << 16) - field<Dword>(voice, 60)) / duration;
        field<Dword>(voice, 108) = field<Dword>(layer, 44);
        field<Dword>(voice, 112) = field<Dword>(layer, 48);
        field<Byte>(voice, 89) = field<Byte>(layer, 27);
        field<Byte>(voice, 90) = field<Byte>(layer, 29);
        field<std::uint16_t>(voice, 116) = field<std::uint16_t>(layer, 84);
        field<Byte>(voice, 91) =
            field<std::int8_t>(layer, 28)
                ? Byte(next_audio_random_value() % Dword(field<std::int8_t>(layer, 28)))
                : 0;
        field<Byte>(voice, 92) =
            field<std::int8_t>(layer, 30) ? Byte(next_audio_random_value() % field<Byte>(layer, 29)) : 0;
        field<std::uint16_t>(voice, 94) = field<std::uint16_t>(parameters, 14);
        field<std::uint16_t>(voice, 124) = field<std::uint16_t>(parameters, 12);
        field<std::uint16_t>(voice, 122) = 0;
        calculate_pitch(index);
        field<std::uint16_t>(voice, 14) = bank_id;
        field<std::uint16_t>(voice, 12) = event;
        field<std::uint16_t>(voice, 24) =
            std::uint16_t(field<std::uint16_t>(parameters, 8) + field<std::uint16_t>(voice, 70));
        field<std::uint16_t>(voice, 26) = field<std::uint16_t>(parameters, 10);
        field<Dword>(voice, 20) = field<Dword>(layer, 96);
        field<Dword>(voice, 16) = field<Dword>(layer, 88);
        field<std::uint16_t>(voice, 28) = field<std::uint16_t>(layer, 86);
        field<Byte>(voice, 30) = field<Byte>(layer, 19);
        field<std::uint16_t>(voice, 32) = std::uint16_t(format);
        field<Byte>(voice, 31) = field<Byte>(layer, 3);
        for (int i = 0; i < field<std::int8_t>(layer, 3); ++i)
            field<std::uint16_t>(voice, 72 + i * 2) = field<std::uint16_t>(layer, 148 + i * 2);
        field<Byte>(voice, 83) = field<Byte>(parameters, 4);
        field<Byte>(voice, 84) = field<Byte>(layer, 22);
        field<Byte>(voice, 85) = Byte(effect);
        calculate_effect(0, index);
        calculate_volume(index);
        if (start_layer(layer, bank, index, field<std::uint16_t>(parameters, 14),
                        field<std::uint16_t>(parameters, 18), field<std::uint16_t>(parameters, 20),
                        field<std::uint16_t>(parameters, 22)) >= 0)
            return result;
        for (int i = 0; i < field<std::int8_t>(layer, 3); ++i)
            retire_voice(field<std::int16_t>(voice, 4 + i * 2));
    }
}
} // namespace nfs::sound::pending
