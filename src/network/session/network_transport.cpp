#include "nfs/network/session/network_runtime.hpp"
extern "C" void __cdecl _assert(const char *, const char *, unsigned);
namespace nfs::network {
Dword *comm_api = nullptr;

Dword destroy_comm_packet(Dword *owner, Dword *packet) {
    if (!packet)
        return 0;
    if (!pending_packets(packet)) {
        using Delete = void(__thiscall *)(Dword *, unsigned);
        reinterpret_cast<const Delete *>(packet[0])[1](packet, 1);
        return 1;
    }
    swap_error(packet, 2);
    return 1;
}
// The assert remains the real CRT failure boundary.
void destroy_packet(Dword *packet) {
    if (!comm_api)
        _assert("gpCommApi", "cmn\\comm_api.cpp", 342);
    destroy_comm_packet(comm_api, packet);
}

void close_route(Dword *owner, void *route) {
    if (!route)
        return;
    auto *first = field<Dword *>(route, 36);
    if (first && read_error(first)) {
        using Close = void(__thiscall *)(Dword *);
        reinterpret_cast<const Close *>(first[0])[20 / 4](first);
        kernel::signal_set(
            reinterpret_cast<kernel::Signal *>(reinterpret_cast<std::uint8_t *>(owner) + 72));
    }
    auto *second = field<Dword *>(route, 40);
    if (second && read_error(second)) {
        using Close = void(__thiscall *)(Dword *);
        reinterpret_cast<const Close *>(second[0])[20 / 4](second);
        kernel::signal_set(
            reinterpret_cast<kernel::Signal *>(reinterpret_cast<std::uint8_t *>(owner) + 72));
    }
    auto *connections = field<Dword *>(owner, 56);
    if (connections) {
        using Remove = void(__thiscall *)(Dword *, Dword *);
        if (second)
            reinterpret_cast<const Remove *>(connections[0])[3](connections, second);
        if (first)
            reinterpret_cast<const Remove *>(connections[0])[3](connections, first);
    }
}
} // namespace nfs::network
