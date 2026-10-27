#include "nfs/network/transport/comm_packet.hpp"
#include <cstring>
extern "C" void __cdecl _assert(const char *, const char *, unsigned);
namespace nfs::network::comm {

Dword header_bytes(Dword *connection) {
    auto *filter = field<Dword *>(connection, 88);
    if (!filter)
        return 0;
    using Get = Dword(__thiscall *)(Dword *);
    return reinterpret_cast<const Get *>(filter[0])[19](filter);
}

Dword send_via_filter(Dword *connection, Packet *packet) {
    if (auto *filter = field<Dword *>(connection, 88)) {
        using Send = Dword(__thiscall *)(Dword *, Packet *);
        return reinterpret_cast<const Send *>(filter[0])[14](filter, packet) & 255;
    }
    return send_via_driver(connection, packet);
}

Dword send_via_driver(Dword *connection, Packet *packet) {
    if (!packet)
        return 0;
    auto *driver = field<Dword *>(connection, 84);
    if (!driver) {
        set_error(48);
        _assert("0", "cmn\\connection2.cpp", 312);
        return 0;
    }
    const int available = available_bytes(packet);
    using Send = Dword(__thiscall *)(Dword *, Dword *, Packet *);
    Dword sent = reinterpret_cast<const Send *>(driver[0])[8](driver, connection, packet);
    if (sent == Dword(available))
        set_status(packet, 3);
    return sent;
}
// Callback includes the connection as its fourth argument.
std::uint8_t send_immediate(Dword *connection, Dword identifier, const void *data, Dword bytes,
                            const char *address, std::uint16_t port) {
    alignas(4) std::uint8_t endpoint[276];
    fill(endpoint, 0, sizeof(endpoint));
    field<Dword>(endpoint, 0) = identifier;
    if (address) {
        std::strncpy(reinterpret_cast<char *>(endpoint) + 4, address, 256);
        field<std::uint16_t>(endpoint, 260) = port;
    }
    auto *packet = static_cast<Packet *>(allocate_object(sizeof(Packet)));
    if (packet)
        construct_packet(packet, endpoint, static_cast<int>(header_bytes(connection)),
                         static_cast<int>(bytes));
    set_status(packet, 1);
    write_packet(packet, data, static_cast<int>(bytes));
    send_via_filter(connection, packet);
    using Event = void(__thiscall *)(Dword *, Dword, Dword, Dword, Dword *);
    bool result = false;
    switch (status(packet)) {
    case 0:
    case 1:
    case 4:
    case 6:
        reinterpret_cast<const Event *>(connection[0])[0](connection, 8, status(packet), identifier,
                                                          connection);
        break;
    case 3:
        result = true;
        reinterpret_cast<const Event *>(connection[0])[0](
            connection, 6, reinterpret_cast<Dword>(packet), identifier, connection);
        break;
    case 5:
        result = true;
        reinterpret_cast<const Event *>(connection[0])[0](
            connection, 7, reinterpret_cast<Dword>(packet), identifier, connection);
        break;
    default:
        _assert("0", "cmn\\connection2.cpp", 501);
        break;
    }
    if (packet)
        packet->methods[0](packet, 1);
    return result ? 1 : 0;
}
} // namespace nfs::network::comm
