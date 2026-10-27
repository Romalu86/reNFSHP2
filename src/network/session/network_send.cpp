#include "nfs/network/transport/network_mailbox.hpp"
#include "nfs/network/transport/comm_packet.hpp"
#include "nfs/network/packets/network_cipher.hpp"
#include "../transport/comm_connection_runtime.hpp"
namespace nfs::network::comm {

Dword send(Dword *connection, const void *data, int bytes, const char *address, std::uint16_t port) {
    if (!data || bytes <= 0)
        return 0;
    if (address)
        return session::pending::send_connection_data(connection, data, bytes, address, port);
    using Send = Dword(__thiscall *)(Dword *, const void *, int);
    return reinterpret_cast<const Send *>(connection[0])[7](connection, data, bytes);
}
} // namespace nfs::network::comm
namespace nfs::network {

Dword send_route(Dword *owner, void *route, const std::uint8_t *data, Dword bytes,
                std::uint8_t flags) {
    const Dword now = transport_time();
    field<Dword>(route, 28) = now;
    field<Dword>(owner, 20) = now;
    const std::uint8_t opcode = data[0];
    field<std::uint8_t>(route, 25) = opcode;
    auto *connection = field<Dword *>(route, 40);
    const char *address = nullptr;
    std::uint16_t port = 0;
    Dword local_server = 0;
    if (!(flags & 4) || !connection)
        connection = field<Dword *>(route, 36);
    else {
        address = reinterpret_cast<const char *>(route) + 44;
        port = field<std::uint16_t>(route, 300);
        local_server = server()[44 / 4];
    }
    Dword sequence = 0;
    if (flags & 16) {
        sequence = field<Dword>(owner, 52);
        ++field<Dword>(owner, 52);
    }
    if (connection) {
        if (!local_server) {
            using Send = void(__thiscall *)(Dword *, const void *, Dword);
            if (protocol(connection) == 1 && opcode != 16 && opcode != 18 && opcode != 8) {
                Dword written;
                if (void *encoded = cipher::encode_locked(reinterpret_cast<Dword>(connection), data,
                                                          bytes, &written)) {
                    reinterpret_cast<const Send *>(connection[0])[7](connection, encoded, written);
                    cipher::release_locked(encoded);
                }
            } else
                reinterpret_cast<const Send *>(connection[0])[7](connection, data, bytes);
        } else
            comm::send(connection, data, static_cast<int>(bytes), address, port);
    }
    kernel::signal_set(
        reinterpret_cast<kernel::Signal *>(reinterpret_cast<std::uint8_t *>(owner) + 72));
    return sequence;
}
} // namespace nfs::network
