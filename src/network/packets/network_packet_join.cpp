#include "nfs/network/packets/network_packet_handlers.hpp"
#include <cstring>
namespace nfs::network::packets::pending {

void reset_lobby() {
    auto *rules = system(72);
    at<Dword>(rules, 168) = 0;
    at<Dword>(rules, 172) = 0;
    at<Dword>(rules, 184) = 0;
    set_game_rule(rules, 14, 0);
    ui::CarSettings empty{};
    for (unsigned slot = 0; slot < 24; ++slot)
        std::memcpy(bytes(rules) + 248 + slot * 84, &empty, sizeof(empty));
}

void send_join(Dword *owner) {
    reset_lobby();
    at<std::int16_t>(owner, 380) = 4;
    alignas(4) Byte packet[92];
    packet[0] = 18;
    at<std::uint16_t>(packet, 2) = at<std::uint16_t>(owner, 72);
    ScopedString name;
    gui::string_copy(&name.value, reinterpret_cast<gui::String *>(bytes(owner) + 4));
    gui::string_make_narrow(&name.value);
    std::strncpy(reinterpret_cast<char *>(packet + 8), gui::string_ascii(&name.value), 24);
    packet[31] = 0;
    std::strncpy(reinterpret_cast<char *>(packet + 32), reinterpret_cast<char *>(owner) + 30,
                 24);
    packet[55] = 0;
    char key[33]{};
    generate_client_key(key, 33);
    std::strncpy(reinterpret_cast<char *>(packet + 56), key, 33);
    packet[88] = 0;
    at<Dword>(packet, 4) = 18088178;
    send(bytes(owner) + 72, packet, sizeof(packet));
}

void reconnect(Dword *owner, Dword address, std::uint16_t port, Dword flags) {
    if (port)
        at<std::uint16_t>(owner, 38) = port;
    auto *connection = at<Dword *>(owner, 44);
    if (!connection)
        return;
    using Stop = void(__thiscall *)(Dword *);
    auto *signal = reinterpret_cast<kernel::Signal *>(bytes(owner) + 72);
    if (read_error(connection)) {
        reinterpret_cast<const Stop *>(connection[0])[5](connection);
        kernel::signal_set(signal);
        kernel::thread_yield(10);
    }
    connection = at<Dword *>(owner, 44);
    using Start = Dword(__thiscall *)(Dword *, Dword, std::uint16_t, Dword);
    if (!(reinterpret_cast<const Start *>(connection[0])[9](connection, address,
                                                            at<std::uint16_t>(owner, 38), flags) &
          255)) {
        swap_error(connection, 1);
        reinterpret_cast<const Stop *>(connection[0])[5](connection);
        kernel::signal_set(signal);
        kernel::thread_yield(10);
    }
    kernel::signal_set(signal);
}
} // namespace nfs::network::packets::pending
