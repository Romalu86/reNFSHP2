#include "../session/network_lifecycle_runtime.hpp"
namespace nfs::network::session::pending {

void send_challenge(Dword *owner, Dword identifier, const char *challenge) {
    alignas(4) Byte packet[12];
    packet[0] = 17;
    std::strcpy(reinterpret_cast<char *>(packet + 1), challenge);
    ui::PlayerRecord player{};
    if (queue::copy_matching(player_queue(), reinterpret_cast<Dword>(&identifier),
                             reinterpret_cast<Dword>(&player), nullptr)) {
        alignas(4) Byte route[304];
        if (queue::copy_matching(nfs::network::server_queue(owner), reinterpret_cast<Dword>(bytes(&player) + 2),
                                 reinterpret_cast<Dword>(route), nullptr))
            packets::send(route, packet, 11);
    }
}
} // namespace nfs::network::session::pending
