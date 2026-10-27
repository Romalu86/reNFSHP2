#include "nfs/audio/backend/sound_ownership.hpp"
namespace nfs::sound::backend {

int stream_buffer_bytes(int blocks) {
    return int(Dword(blocks) * 24 + 100);
}
// Empty platform prefix.
int stream_platform_bytes() {
    return 0;
}
// Bytes 33 C0 C3; arguments are not read.
int initialize_stream_platform(int, void *) {
    return 0;
}
// Bytes 33 C0 C3; arguments are not read.
int release_stream_platform(int) {
    return 0;
}

int stream_total_bytes(int blocks) {
    return stream_platform_bytes() + stream_buffer_bytes(blocks);
}

int create_stream_queue(Dword on_block, Dword on_consumed, Dword context, void *memory, Dword bytes) {
    if (!control.initialized)
        return -10;
    lock();
    for (unsigned i = 0; i < pending::backend_stream_slot_count; ++i) {
        if (pending::stream_nodes[i])
            continue;
        if (initialize_stream_platform(i, memory) < 0) {
            unlock();
            return -6;
        }
        auto *node =
            reinterpret_cast<Dword *>(static_cast<Byte *>(memory) + stream_platform_bytes());
        const auto prefix = stream_platform_bytes();
        field<void *>(node, 40) = memory;
        field<std::uint16_t>(node, 24) = std::uint16_t((bytes - Dword(prefix) - 100) / 24);
        node[11] = on_block;
        node[12] = on_consumed;
        node[13] = context;
        node[0] = 4294967295u;
        pending::stream_nodes[i] = node;
        unlock();
        return i;
    }
    unlock();
    return -9;
}

int queue_stream_block(int slot, const void *block) {
    if (!control.initialized)
        return -10;
    auto *node = pending::stream_nodes[slot];
    if (field<std::int16_t>(node, 16) >= field<std::int16_t>(node, 24) - 1)
        return -13;
    auto *entry = reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(node) + 76 +
                                           field<std::int16_t>(node, 30) * 24);
    entry[1] = field<Dword>(block, 4);
    entry[0] = node[1];
    for (unsigned channel = 0; channel < field<Byte>(node, 58); ++channel) {
        entry[2 + channel] = field<Dword>(block, 12 + channel * 4);
        ++field<std::uint16_t>(node, 16 + channel * 2);
    }
    node[8] += field<Dword>(block, 4) & 2147483647;
    Dword sequence = node[1]++;
    ++field<std::uint16_t>(node, 30);
    field<Byte>(node, 27) = 1;
    if (field<std::int16_t>(node, 30) >= field<std::int16_t>(node, 24))
        field<std::uint16_t>(node, 30) = 0;
    return int(sequence);
}

int stream_free_blocks(int slot) {
    if (!control.initialized)
        return -10;
    auto *node = pending::stream_nodes[slot];
    return field<std::int16_t>(node, 24) - field<std::int16_t>(node, 16) - 1;
}

int stop_stream_queue(int slot) {
    if (!control.initialized)
        return -10;
    auto *node = pending::stream_nodes[slot];
    stop(*node);
    dispatch_stream_events();
    *node = 4294967295u;
    for (unsigned c = 0; c < field<Byte>(node, 58); ++c)
        if (node[15 + c])
            release(reinterpret_cast<void *>(node[15 + c]));
    return 0;
}

int release_stream_queue(int slot) {
    if (!control.initialized)
        return -10;
    release_stream_platform(slot);
    pending::stream_nodes[slot] = nullptr;
    return 0;
}

int read_stream_block(int slot, int channel, int *samples, int *last) {
    auto *node = pending::stream_nodes[slot];
    auto advance_event = [&]() {
        auto cursor = field<std::int16_t>(node, 28);
        if (node[11]) {
            auto *event = deferred_events + deferred_count++ * 8;
            field<std::uint16_t>(event, 0) = 1;
            field<std::uint16_t>(event, 2) = std::uint16_t(slot);
            field<Dword>(event, 4) = field<Dword>(node, 84 + cursor * 24);
        }
        ++field<std::uint16_t>(node, 28);
        if (field<std::int16_t>(node, 28) >= field<std::int16_t>(node, 24))
            field<std::uint16_t>(node, 28) = 0;
    };
    auto &cursor = field<std::int16_t>(node, 8 + channel * 2);
    auto &queued = field<std::int16_t>(node, 16 + channel * 2);
    bool primary = channel == field<std::int8_t>(node, 26);
    if (!queued) {
        if (primary && !field<Byte>(node, 27) && cursor != field<std::int16_t>(node, 28))
            advance_event();
        return 0;
    }
    auto *entry = reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(node) + 76 + cursor * 24);
    const auto count = int(entry[1] << 1) >> 1;
    *samples = count;
    *last = int(entry[1]) >> 31;
    if (primary) {
        if (cursor != field<std::int16_t>(node, 28))
            advance_event();
        node[8] -= Dword(count);
        node[9] += Dword(count);
    }
    ++cursor;
    if (cursor >= field<std::int16_t>(node, 24))
        cursor = 0;
    --queued;
    if (!queued && primary)
        field<Byte>(node, 27) = 0;
    return entry[2 + channel] ? int(entry[2 + channel]) : -1;
}

void *consume_stream_samples(int slot, int channel, int samples) {
    auto *node = pending::stream_nodes[slot];
    void *result = node;
    if (channel == field<std::int8_t>(node, 26)) {
        node[9] -= Dword(samples);
        if (node[12]) {
            auto *event = deferred_events + deferred_count++ * 8;
            field<std::uint16_t>(event, 0) = 0;
            field<std::uint16_t>(event, 2) = std::uint16_t(slot);
            field<Dword>(event, 4) = Dword(samples);
            result = event;
        }
    }
    return result;
}

void dispatch_stream_events() {
    using Block = void(__cdecl *)(Dword, Dword);
    using Consumed = void(__cdecl *)(Dword, Dword, Dword);
    for (int i = 0; i < deferred_count; ++i) {
        const auto *event = deferred_events + i * 8;
        Dword slot = field<std::uint16_t>(event, 2);
        auto *node = pending::stream_nodes[slot];
        if (field<std::int16_t>(event, 0) == 0)
            reinterpret_cast<Consumed>(node[12])(slot, field<Dword>(event, 4), node[13]);
        else
            reinterpret_cast<Block>(node[11])(field<Dword>(event, 4), node[13]);
    }
    deferred_count = 0;
}

int largest_voice_channel(int index, int channels) {
    int selected = -1;
    std::int16_t largest = -1;
    for (int c = 0; c < channels; ++c) {
        auto v = field<std::int16_t>(control.voices + index, 4 + c * 2);
        if (largest < v) {
            selected = c;
            largest = v;
        }
    }
    return selected;
}

int find_voice_channel(int index) {
    auto parent = field<std::int16_t>(control.voices + index, 36);
    if (parent != -1)
        for (int c = 1; c < 4; ++c)
            if (field<std::int16_t>(control.voices + parent, 4 + c * 2) == index)
                return c;
    return 0;
}
} // namespace nfs::sound::backend
