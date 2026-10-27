#include "nfs/network/session/network_runtime.hpp"
#include <cstring>
namespace nfs::network {

Dword protocol(void *object) {
    return field<Dword>(object, 16);
}

void *address(void *object) {
    return static_cast<std::uint8_t *>(object) + 372;
}

Dword pending_packets(void *object) {
    return field<Dword>(object, 84);
}

Dword swap_error(void *object, Dword error) {
    const Dword previous = field<Dword>(object, 68);
    field<Dword>(object, 68) = error;
    return previous;
}

Dword read_error(void *object) {
    return field<Dword>(object, 68);
}
// There are two stack arguments; adjusted ECX is unused.
Dword transport_matches_route(void *connection, void *route) {
    Dword candidate;
    if (protocol(connection) == 1)
        candidate = field<Dword>(route, 36);
    else {
        if (protocol(connection) != 2)
            return 0;
        candidate = field<Dword>(route, 40);
        if (!candidate)
            return 0;
        if (std::strcmp(static_cast<char *>(connection) + 112, static_cast<char *>(route) + 44))
            return 0;
        const auto *port = static_cast<const std::uint8_t *>(address(connection)) + 2;
        const unsigned host_port = (unsigned(port[0]) << 8) | port[1];
        if (field<std::uint16_t>(route, 300) != host_port)
            return 0;
    }
    return candidate && candidate == reinterpret_cast<Dword>(connection);
}
} // namespace nfs::network
