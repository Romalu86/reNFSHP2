#include "nfs/network/transport/comm_packet.hpp"
extern "C" void __cdecl _assert(const char *, const char *, unsigned);
namespace nfs::network::comm {

Dword initialize_packet(Packet *packet) {
    fill(&packet->buffer, 0, 24);
    fill(packet->endpoint, 0, 276);
    packet->mode = 0;
    packet->references = 0;
    packet->status = 0;
    return 1;
}

Packet *construct_packet(Packet *packet, const void *endpoint, int header, int payload) {
    initialize_kind(reinterpret_cast<Dword *>(packet) + 1, 4);
    packet->methods = packet_methods;
    initialize_packet(packet);
    allocate_buffer(packet, header, payload);
    if (endpoint)
        set_endpoint(packet, endpoint);
    return packet;
}

Packet *construct_packet_copy(Packet *packet, Packet *source) {
    initialize_kind(reinterpret_cast<Dword *>(packet) + 1, 4);
    packet->methods = packet_methods;
    initialize_packet(packet);
    if (source)
        copy_packet(packet, source);
    return packet;
}

void destroy_packet(Packet *packet) {
    packet->methods = packet_methods;
    release_buffer(packet);
}

void *__fastcall delete_packet(Packet *packet, void *, unsigned flags) {
    destroy_packet(packet);
    if (flags & 1)
        kernel::memory_free(packet);
    return packet;
}
const gui::Destroy packet_methods[]{reinterpret_cast<gui::Destroy>(delete_packet)};

Packet *copy_packet(Packet *packet, Packet *source) {
    release_buffer(packet);
    Buffer *buffer = packet_buffer(source);
    if (buffer->data && buffer->capacity > 0) {
        allocate_buffer(packet, buffer->header_bytes, buffer->payload_bytes);
        write_packet(packet, buffer->data, buffer->capacity);
    }
    set_endpoint(packet, packet_endpoint(source));
    return packet;
}
// Share the packet buffer and endpoint, then retain the source.
Packet *share_packet(Packet *packet, Packet *source) {
    if (source) {
        release_buffer(packet);
        copy_buffer(packet, packet_buffer(source));
        set_endpoint(packet, packet_endpoint(source));
        add_reference(source);
    }
    return packet;
}

Buffer *packet_buffer(Packet *packet) {
    return &packet->buffer;
}
// Copies the six descriptor words in forward order.
bool copy_buffer(Packet *packet, const Buffer *source) {
    if (!source)
        return false;
    auto *out = reinterpret_cast<Dword *>(&packet->buffer);
    auto *in = reinterpret_cast<const Dword *>(source);
    for (unsigned i = 0; i < 6; ++i)
        out[i] = in[i];
    return true;
}

void set_mode(Packet *packet, int value) {
    if (packet->mode != value) {
        packet->mode = value;
        reset_cursor(packet);
    }
}

int mode(Packet *packet) {
    return packet->mode;
}
// The allocation is (capacity+4)&~3, including aligned sizes.
Dword allocate_buffer(Packet *packet, int header, int payload) {
    fill(&packet->buffer, 0, 24);
    packet->buffer.header_bytes = header;
    packet->buffer.payload_bytes = payload;
    const Dword capacity = Dword(header) + Dword(payload);
    packet->buffer.capacity = static_cast<int>(capacity);
    if (!capacity)
        return 0;
    const Dword bytes = (capacity + 4) & ~Dword(3);
    packet->buffer.data = static_cast<std::uint8_t *>(allocate_named("PACKET::buffer", bytes, 1));
    fill(packet->buffer.data, 0, bytes);
    packet->buffer.owner = packet;
    clear_packet(packet);
    return 1;
}

Dword release_buffer(Packet *packet) {
    if (packet->buffer.data) {
        if (packet->buffer.owner == packet) {
            kernel::memory_free(packet->buffer.data);
            fill(&packet->buffer, 0, 24);
            return 1;
        }
        if (packet->buffer.owner)
            release_reference(packet->buffer.owner);
    }
    fill(&packet->buffer, 0, 24);
    return 1;
}

void clear_packet(Packet *packet) {
    if (packet->buffer.data && packet->buffer.capacity > 0)
        fill(packet->buffer.data, 0, packet->buffer.capacity);
    reset_cursor(packet);
}
// Only AL is a boolean; retained upper capacity bytes are explicit.
Dword bind_buffer(Packet *packet, void *data, int header, int payload, Packet *owner) {
    packet->buffer.data = static_cast<std::uint8_t *>(data);
    packet->buffer.header_bytes = header;
    packet->buffer.payload_bytes = payload;
    const Dword capacity = Dword(header) + Dword(payload);
    packet->buffer.capacity = static_cast<int>(capacity);
    packet->buffer.owner = owner;
    packet->buffer.cursor = 0;
    return (capacity & 4294967040u) | 1;
}

void *packet_data(Packet *packet) {
    if (!packet->buffer.data)
        return nullptr;
    return packet->mode == 1 ? packet->buffer.data : packet->buffer.data + packet->buffer.cursor;
}

int available_bytes(Packet *packet) {
    return packet->mode == 1
               ? packet->buffer.cursor
               : static_cast<int>(Dword(packet->buffer.capacity) - Dword(packet->buffer.cursor));
}

bool set_extent(Packet *packet, int bytes) {
    bool fits = true;
    if (packet->buffer.capacity < bytes) {
        bytes = packet->buffer.capacity;
        fits = false;
    }
    seek_packet(packet, bytes, packet->mode == 1 ? 0 : 2);
    return fits;
}

int seek_packet(Packet *packet, int offset, int origin) {
    if (origin == 1)
        offset = static_cast<int>(Dword(packet->buffer.cursor) + Dword(offset));
    else if (origin == 2)
        offset = static_cast<int>(Dword(packet->buffer.capacity) - Dword(offset));
    packet->buffer.cursor = offset;
    if (packet->buffer.cursor < 0) {
        packet->buffer.cursor = 0;
        return packet->buffer.cursor;
    }
    if (packet->buffer.capacity < packet->buffer.cursor)
        packet->buffer.cursor = packet->buffer.capacity;
    return packet->buffer.cursor;
}

void *packet_endpoint(Packet *packet) {
    return packet->endpoint;
}

bool set_endpoint(Packet *packet, const void *source) {
    if (!source)
        return false;
    auto *out = reinterpret_cast<Dword *>(packet->endpoint);
    auto *in = static_cast<const Dword *>(source);
    for (unsigned i = 0; i < 69; ++i)
        out[i] = in[i];
    return true;
}

void reset_cursor(Packet *packet) {
    seek_packet(packet, 0, packet->mode == 1 ? 0 : 2);
}

int reference_count(Packet *packet) {
    return packet->references;
}

int add_reference(Packet *packet) {
    packet->references = static_cast<int>(Dword(packet->references) + 1);
    return packet->references;
}

int release_reference(Packet *packet) {
    packet->references = static_cast<int>(Dword(packet->references) - 1);
    if (packet->references < 0)
        _assert("0", "cmn\\packet.cpp", 631);
    return packet->references;
}

void set_status(Packet *packet, Dword value) {
    packet->status = value;
}

Dword status(Packet *packet) {
    return packet->status;
}
} // namespace nfs::network::comm
