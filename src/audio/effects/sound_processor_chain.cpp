#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::backend {
// Equal priorities insert before the existing processor.
void insert_processor(void **head, void *node) {
    void *cursor = *head, *previous = nullptr;
    while (cursor) {
        if (field<std::uint16_t>(node, 24) <= field<std::uint16_t>(cursor, 24))
            break;
        previous = cursor;
        cursor = field<void *>(cursor, 8);
    }
    field<void *>(node, 8) = cursor;
    if (cursor)
        field<void *>(cursor, 16) = node;
    if (!previous)
        *head = node;
    else {
        field<void *>(previous, 8) = node;
        field<void *>(node, 16) = previous;
    }
}
// The saved head removal leaves the new head's back-link untouched.
void remove_processor(void **head, void *node) {
    void *cursor = *head;
    if (cursor == node) {
        *head = field<void *>(cursor, 8);
        return;
    }
    while (field<void *>(cursor, 8) && field<void *>(cursor, 8) != node)
        cursor = field<void *>(cursor, 8);
    void *next = field<void *>(cursor, 8);
    if (next && next == node) {
        if (field<void *>(next, 8))
            field<void *>(field<void *>(next, 8), 16) = cursor;
        field<void *>(cursor, 8) = field<void *>(next, 8);
    }
}

void *find_stream(int key) {
    if (stream_bindings && (stream_limits & 65535))
        for (unsigned i = 0; i < (stream_limits & 65535); ++i)
            if (int(stream_bindings[i * 2]) == key)
                return stream_bindings + i * 2;
    return nullptr;
}

int root_voice(int index) {
    int parent = field<std::int16_t>(control.voices + index, 36);
    return parent != -1 ? parent : index;
}

int stream_slot(int index) {
    for (unsigned i = 0; i < pending::backend_stream_slot_count; ++i)
        if (pending::stream_nodes[i] &&
            find_voice(*pending::stream_nodes[i]) == index)
            return int(i);
    return -1;
}

void fill_pan(Dword key, void *output) {
    for (unsigned channel = 0; channel < Byte(output_channels); ++channel)
        field<std::int16_t>(output, channel * 2) =
            std::int16_t(pending::pan_table[((key >> 8) & 255) * 4 + channel] * 258);
}

void set_mix_effect(int index, int channel, float value) {
    if (pending::mix_format[channel + 3] == 2)
        value = float(static_cast<long double>(value) * 1.5f);
    field<float>(mix_voices + index, 40 + channel * 4) = value;
    field<Byte>(mix_voices + index, 1) = 1;
}

void activate_mix_voice(int index) {
    auto *voice = mix_voices + index;
    field<Dword>(voice, 44) = 0;
    field<Byte>(voice, 1) = 0;
    field<Dword>(voice, 36) = field<Dword>(voice, 40);
    for (unsigned channel = 0; channel < pending::mix_modes; ++channel)
        field<Dword>(voice, 4 + channel * 4) = field<Dword>(voice, 20 + channel * 4);
    field<Byte>(voice, 0) = 2;
}

void destroy_mix_voice(int index) {
    auto *voice = mix_voices + index;
    field<Byte>(voice, 0) = 0;
    while (void *node = field<void *>(voice, 48)) {
        void *next = field<void *>(node, 8);
        if (auto destroy = field<Dword>(node, 4))
            reinterpret_cast<void(__cdecl *)(void *)>(destroy)(node);
        release(field<void *>(voice, 48));
        field<void *>(voice, 48) = next;
    }
    for (unsigned offset : {56u, 60u, 64u, 72u, 76u})
        field<Dword>(voice, offset) = 0;
}
} // namespace nfs::sound::backend
