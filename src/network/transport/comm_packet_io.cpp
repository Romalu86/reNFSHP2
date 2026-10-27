#include "nfs/network/transport/comm_packet.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
extern "C" void __cdecl _assert(const char *, const char *, unsigned);
namespace nfs::network::comm {

int write_packet(Packet *packet, const void *source, int bytes) {
    if (bytes < 1)
        return 0;
    auto *data = packet->buffer.data;
    if (!data) {
        set_error(41);
        _assert("0", "cmn\\packet.cpp", 443);
        return 0;
    }
    const int cursor = packet->buffer.cursor;
    if (packet->mode == 1) {
        if (static_cast<int>(Dword(packet->buffer.capacity) - Dword(cursor)) < bytes) {
            set_error(43);
            _assert("0", "cmn\\packet.cpp", 455);
            return 0;
        }
        if (source)
            kernel::memory_copy(data + cursor, source, bytes);
        packet->buffer.cursor = static_cast<int>(Dword(packet->buffer.cursor) + Dword(bytes));
        return bytes;
    }
    if (cursor < bytes) {
        set_error(43);
        _assert("0", "cmn\\packet.cpp", 472);
        return 0;
    }
    packet->buffer.cursor = static_cast<int>(Dword(cursor) - Dword(bytes));
    if (source)
        kernel::memory_copy(data + packet->buffer.cursor, source, bytes);
    return bytes;
}

int read_packet(Packet *packet, void *destination, int bytes) {
    if (bytes < 1)
        return 0;
    auto *data = packet->buffer.data;
    if (!data) {
        set_error(41);
        return 0;
    }
    const int available = available_bytes(packet);
    if (available < bytes)
        bytes = available;
    if (packet->mode == 1) {
        packet->buffer.cursor = static_cast<int>(Dword(packet->buffer.cursor) - Dword(bytes));
        if (destination)
            kernel::memory_copy(destination, data + packet->buffer.cursor, bytes);
    } else {
        if (destination)
            kernel::memory_copy(destination, data + packet->buffer.cursor, bytes);
        packet->buffer.cursor = static_cast<int>(Dword(packet->buffer.cursor) + Dword(bytes));
    }
    return bytes;
}
} // namespace nfs::network::comm
