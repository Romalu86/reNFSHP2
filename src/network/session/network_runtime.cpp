#include "nfs/network/session/network_runtime.hpp"
#include "nfs/network/session/network_gui.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include <mmsystem.h>
#include <cstring>
namespace nfs::network {

void send_packet(Dword route, Dword data, Dword bytes, Dword flags) {
    auto *record = reinterpret_cast<Dword *>(route);
    if (!record[1]) {
        send_route(transport, reinterpret_cast<void *>(route),
                   reinterpret_cast<const std::uint8_t *>(data), bytes,
                   static_cast<std::uint8_t>(flags));
        return;
    }
    if (record[2]) {
        dispatch_server_packet(server(), nullptr, route, data, bytes);
        return;
    }
    dispatch_client_packet(client(), nullptr, route, data, bytes);
}

void disconnect_route(void *route) {
    if (!field<Dword>(route, 4)) {
        close_route(transport, route);
        return;
    }
    while (remove_server_client(server(), nullptr, field<std::uint16_t>(route, 0))) {
    }
    lifecycle::construction::network::queue::erase(server_queue(server()),
                                                   reinterpret_cast<Dword>(route), nullptr);
    field<std::uint16_t>(client(), 380) = 1;
}

void disconnect_client(Dword *owner, Dword reason) {
    if (!field<Dword>(owner, 84))
        return;
    field<std::uint16_t>(owner, 380) = 10;
    field<Dword>(owner, 56) = reason;
    disconnect_route(reinterpret_cast<std::uint8_t *>(owner) + 72);
    const Dword deadline = transport_time() + 7500;
    while (field<std::int16_t>(owner, 380) == 10 &&
           static_cast<int>(transport_time() - deadline) < 0)
        kernel::thread_yield(1);
    std::memset(reinterpret_cast<std::uint8_t *>(owner) + 72, 0, 304);
    gui::String notice;
    gui::string_construct_reference(&notice, 1703);
    try {
        Dword *game = network_game();
        if (auto *frontend = field<Dword *>(game, 92))
            ui::notice(frontend, nullptr, &notice, -1, 0, 0, 0);
    } catch (...) {
        gui::string_destruct(&notice);
        throw;
    }
    gui::string_destruct(&notice);
    field<std::uint16_t>(owner, 2096) = 0;
    field<std::uint16_t>(owner, 28) = 0;
    field<Dword>(owner, 24) = 0;
}
// Bytes 1..3 of packet and 2..3 of descriptor were unwritten.
void forward(Dword *owner, Dword target, Dword message, Dword flags) {
    std::uint8_t packet[140];
    packet[0] = 4;
    std::memcpy(packet + 4, reinterpret_cast<void *>(message), 136);
    if (target) {
        send_packet(target, reinterpret_cast<Dword>(packet), sizeof(packet), flags);
        return;
    }
    struct Descriptor {
        std::uint16_t type, padding;
        Dword data, bytes, flags, target;
    } descriptor;
    descriptor.type = 1;
    descriptor.data = reinterpret_cast<Dword>(packet);
    descriptor.bytes = sizeof(packet);
    descriptor.flags = flags;
    descriptor.target = 0;
    Dword *visitor = owner ? reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(owner) +
                                                     reinterpret_cast<Dword *>(owner[1])[2] + 4)
                          : nullptr;
    lifecycle::construction::network::queue::visit(
        server_queue(owner), 0, reinterpret_cast<Dword>(&descriptor), visitor, 0, nullptr);
}
} // namespace nfs::network
