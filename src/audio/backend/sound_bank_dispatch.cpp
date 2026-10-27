#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::sound {
namespace pending {
Dword voice_group{};
}

int bank_status(int bank) {
    if (bank >= 0 && bank < static_cast<int>(pending::bank_limits & 65535) &&
        control.banks[bank].data)
        return control.banks[bank].loading ? -18 : 0;
    return -8;
}
// The signed upper bound does not reject negative indices.
void *bank_entry(void *bank, int event) {
    if (event >= field<std::uint16_t>(bank, 6))
        return nullptr;
    Dword *entry = reinterpret_cast<Dword *>(static_cast<Byte *>(bank) + 20 + Dword(event) * 4);
    return *entry ? reinterpret_cast<void *>(reinterpret_cast<Dword>(entry) + *entry) : nullptr;
}

Dword play_bank(int bank, int event, const VoiceParameters *parameters) {
    if (!control.initialized)
        return Dword(-10);
    if (bank_status(bank) >= 0) {
        void *data = control.banks[bank].data;
        if (event >= 0 && event < field<std::uint16_t>(data, 6))
            return Dword(play_layers(data, bank_entry(data, event), bank, event, parameters));
    }
    return Dword(-8);
}
// Only the low byte of the shared Word is changed.
Byte next_voice_group() {
    Byte candidate = Byte(pending::voice_group);
    const int count = static_cast<std::int16_t>(control.voice_count);
    for (;;) {
        ++candidate;
        if (!candidate)
            candidate = 1;
        pending::voice_group = (pending::voice_group & 4294967040u) | candidate;
        int voice = 0;
        for (; voice < count; ++voice)
            if (field<Byte>(control.voices + voice, 93) &&
                field<Byte>(control.voices + voice, 34) == candidate)
                break;
        if (voice == count || count < 0)
            return candidate;
    }
}

int random_variation(int extent) {
    if (extent < 0)
        extent = 0;
    else if (extent > 65536)
        extent = 65536;
    return static_cast<int>(((pending::next_audio_random_value() & 32767) - 16384) * Dword(extent)) >> 14;
}
// Filter callbacks consume layer records in reverse order.
void dispatch_layer_callbacks(void *layer, Dword mode) {
    while (field<std::int8_t>(layer, 23) > 0) {
        const int index = field<std::int8_t>(layer, 23) - 1;
        field<Byte>(layer, 23) = Byte(index);
        Dword record[3] = {mode, field<Dword>(layer, 52 + index * 4),
                          field<Dword>(layer, 68 + index * 4)};
        for (int callback = 0; callback < control.filter_count; ++callback) {
            using Filter = void(__cdecl *)(Dword *);
            reinterpret_cast<Filter>(control.filters[callback])(record);
        }
    }
}
// Layer selection, callback submission and group assignment.
int play_layers(void *bank, void *entry, int bank_id, int event,
                const VoiceParameters *parameters) {
    int handles[12], count = 0, result = -9;
    Voice *last = nullptr;
    int first_variation = 0;
    bool first = true;
    if (!entry)
        return -8;
    if (!bank)
        bank = entry;
    Byte *cursor = static_cast<Byte *>(entry) + 4;
    const int key = field<std::int8_t>(parameters, 2), velocity = field<std::int8_t>(parameters, 3);
    const Byte group = next_voice_group();
    int more;
    do {
        alignas(4) Byte layer[160];
        more = pending::decode_layers(reinterpret_cast<void **>(&cursor), layer);
        if (first) {
            if (field<std::int16_t>(layer, 0))
                first_variation = random_variation(field<std::int16_t>(layer, 0));
            first = false;
        }
        if (field<std::int8_t>(layer, 8) <= velocity && velocity <= field<std::int8_t>(layer, 9) &&
            field<std::int8_t>(layer, 10) <= key && key <= field<std::int8_t>(layer, 11)) {
            result =
                pending::create_layer(std::uint16_t(bank_id), std::uint16_t(event), bank,
                                               parameters, layer, key, velocity, first_variation);
            if (result < 0) {
                for (int i = 0; i < count; ++i)
                    stop(Dword(handles[i]));
                return result;
            }
            handles[count++] = result;
            dispatch_layer_callbacks(layer, 1);
        }
    } while (more);
    int active = 0;
    for (int i = 0; i < count; ++i)
        if (!voice_missing(Dword(handles[i])))
            handles[active++] = handles[i];
    if (!active)
        return result;
    if (active == 1) {
        int index = find_voice(Dword(handles[0]));
        if (index >= 0) {
            field<Byte>(control.voices + index, 34) = 0;
            field<Byte>(control.voices + index, 35) = 0;
        }
        return result;
    }
    for (int i = 0; i < active; ++i) {
        int index = find_voice(Dword(handles[i]));
        if (index >= 0) {
            last = control.voices + index;
            field<Byte>(last, 34) = group;
            field<Byte>(last, 35) = 0;
        }
    }
    field<Byte>(last, 35) = 1;
    return result;
}
} // namespace nfs::sound
