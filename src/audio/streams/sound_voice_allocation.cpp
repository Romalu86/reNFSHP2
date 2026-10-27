#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::sound::pending {
std::int16_t selected_voices[4]{};
Dword voice_sequence{};
extern Byte &allow_equal_priority, &backend_count, &reserved_voice_count;

int voice_selected(int voice, int count) {
    for (int i = 0; i < count; ++i)
        if (selected_voices[i] == voice)
            return 1;
    return 0;
}
// Free voices first, then priority/age stealing, then ordered grouping.
int allocate_layers(Dword channels, Dword priority, int *handle, int first, int end) {
    for (int i = 0; i < static_cast<int>(channels); ++i)
        selected_voices[i] = -1;
    voice_sequence += 256;
    if (static_cast<int>(voice_sequence) < 0)
        voice_sequence = 0;
    Dword count = 0;
    for (int selection = 0; selection < static_cast<int>(channels); ++selection) {
        int best = -1;
        Dword age = Dword(-1);
        for (int i = first; i < end; ++i) {
            Voice *voice = control.voices + i;
            if (!field<Byte>(voice, 93) && !voice_selected(i, int(count)) &&
                field<Dword>(voice, 40) < age) {
                age = field<Dword>(voice, 40);
                best = i;
            }
        }
        if (best >= 0)
            selected_voices[count++] = std::int16_t(best);
    }
    for (Dword selection = count; static_cast<int>(selection) < static_cast<int>(channels);
         ++selection) {
        Dword limit = priority - (allow_equal_priority ? 0 : 1), age = Dword(-1);
        int best = -1;
        for (int i = first; i < end; ++i) {
            Voice *voice = control.voices + i;
            if (voice_selected(i, int(count)) || field<Byte>(voice, 38) >= 101)
                continue;
            Dword value = field<Byte>(voice, 38);
            if (static_cast<int>(value) < static_cast<int>(limit) ||
                (value == limit && field<Dword>(voice, 40) < age)) {
                best = i;
                limit = value;
                age = field<Dword>(voice, 40);
            }
        }
        if (best >= 0) {
            selected_voices[count++] = std::int16_t(best);
            if (count >= channels)
                break;
        }
    }
    if (count != channels)
        return -9;
    bool swapped;
    do {
        swapped = false;
        for (int i = 0; i < static_cast<int>(channels - 1); ++i)
            if (selected_voices[i + 1] < selected_voices[i]) {
                std::int16_t value = selected_voices[i];
                selected_voices[i] = selected_voices[i + 1];
                selected_voices[i + 1] = value;
                swapped = true;
            }
    } while (swapped);
    const int root = selected_voices[0];
    *handle = static_cast<int>(Dword(root) | voice_sequence);
    for (int i = 0; i < static_cast<int>(count); ++i) {
        Voice *voice = control.voices + selected_voices[i];
        int old = field<int>(voice, 0);
        if (field<Byte>(voice, 93)) {
            if (old < 0)
                old = field<int>(control.voices + field<std::int16_t>(voice, 36), 0);
            stop(Dword(old));
        }
        field<Byte>(voice, 93) = 1;
        field<Dword>(voice, 40) = control.unknown_c;
        field<Byte>(voice, 38) = Byte(priority);
    }
    Voice *primary = control.voices + root;
    field<int>(primary, 0) = *handle;
    field<std::int16_t>(primary, 4) = std::int16_t(root);
    field<std::int16_t>(primary, 36) = -1;
    for (int i = 1; i < static_cast<int>(count); ++i) {
        const int voice = selected_voices[i];
        field<std::int16_t>(primary, 4 + i * 2) = std::int16_t(voice);
        field<Dword>(control.voices + voice, 0) = Dword(-1);
        field<std::int16_t>(control.voices + voice, 36) = std::int16_t(root);
    }
    return root;
}
} // namespace nfs::sound::pending
