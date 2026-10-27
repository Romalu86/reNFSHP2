#include "nfs/audio/music/music_clients.hpp"
#include "nfs/audio/backend/sound_ownership.hpp"
#include <cstring>
namespace nfs::sound {
namespace pending {
std::uint16_t next_layer_format(int *, void *);
}

int find_bank_slot() {
    for (int index = 0; index < int(pending::bank_limits & 65535); ++index)
        if (!control.banks[index].data)
            return index;
    return -9;
}
// The API directly indexes the bank after its initialized check.
int bank_bytes(int bank) {
    if (!control.initialized)
        return -10;
    return int(control.banks[bank].data[2] + control.banks[bank].data[3]);
}

int relocate_bank(void *destination, int bank) {
    if (!control.initialized)
        return -10;
    int bytes = bank_bytes(bank);
    if (bytes < 0)
        return bytes;
    std::memmove(destination, control.banks[bank].data, bytes);
    control.banks[bank].data = static_cast<Dword *>(destination);
    return 0;
}

int bind_bank_layer(void *layer, void *bank) {
    void *sources[4];
    for (int channel = 0; channel < field<std::int8_t>(layer, 3); ++channel)
        sources[channel] = static_cast<Byte *>(bank) + field<Dword>(layer, 104 + channel * 4);
    int cursor = 0;
    for (auto format = pending::next_layer_format(&cursor, layer); format;
         format = pending::next_layer_format(&cursor, layer)) {
        if ((format & 1) && field<Byte>(layer, 3) == 1) {
            void *owner = field<void *>(layer, 156);
            void *binding = backend::bind_sample_owner(owner);
            int slot = backend::create_sample_buffer(
                field<Byte>(layer, 19), field<std::uint16_t>(layer, 86),
                field<Dword>(layer, 88), field<int>(layer, 92), field<int>(layer, 96),
                sources[0]);
            field<int>(binding, 4) = slot;
            if (!slot)
                backend::unbind_sample_owner(owner);
        }
    }
    return 8;
}

int release_bank_layer(void *layer) {
    void *owner = field<void *>(layer, 156);
    void *binding = backend::find_stream(int(reinterpret_cast<Dword>(owner)));
    if (binding && field<int>(binding, 4)) {
        backend::unbind_sample_owner(owner);
        return backend::release_sample_buffer(field<int>(binding, 4));
    }
    return 0;
}

int release_bank(int bank) {
    if (!control.initialized)
        return -10;
    if (bank == -1) {
        for (int index = 0; index < int(pending::bank_limits & 65535); ++index)
            release_bank(index);
        return 0;
    }
    if (bank_status(bank))
        return -8;
    lock();
    void *data = control.banks[bank].data;
    for (int voice = 0; voice < field<std::int16_t>(&control, 8); ++voice)
        if (field<std::int16_t>(control.voices + voice, 14) == bank)
            stop(field<Dword>(control.voices + voice, 0));
    for (int event = 0; event < field<std::uint16_t>(data, 6); ++event) {
        void *entry = bank_entry(data, event);
        if (entry) {
            void *cursor = static_cast<Byte *>(entry) + 4;
            int more;
            do {
                alignas(4) Byte layer[160];
                more = pending::decode_layers(&cursor, layer);
                release_bank_layer(layer);
                dispatch_layer_callbacks(layer, 2);
            } while (more);
        }
    }
    control.banks[bank].data = nullptr;
    control.banks[bank].loading = 0;
    unlock();
    return 0;
}
// The saved negative slot result is not guarded by this function.
int initialize_bank(int *handle, void *data) {
    if (!control.initialized)
        return -10;
    control.shutdown[3] = reinterpret_cast<Callback>(release_bank);
    int bank = find_bank_slot();
    *handle = bank;
    Bank *binding = control.banks + bank;
    binding->data = static_cast<Dword *>(data);
    binding->loading = 1;
    for (int event = 0; event < field<std::uint16_t>(data, 6); ++event) {
        void *entry = bank_entry(data, event);
        if (entry) {
            void *cursor = static_cast<Byte *>(entry) + 4;
            int more;
            do {
                alignas(4) Byte layer[160];
                more = pending::decode_layers(&cursor, layer);
                bind_bank_layer(layer, data);
                dispatch_layer_callbacks(layer, 0);
            } while (more);
        }
    }
    binding->loading = 0;
    return 8;
}
} // namespace nfs::sound
