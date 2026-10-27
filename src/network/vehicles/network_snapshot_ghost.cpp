#include "remote_car_runtime.hpp"
namespace nfs::network::snapshots {

void send_ghost_period(void *client, std::uint16_t identifier, std::uint8_t frames) {
    struct GhostPeriod {
        std::uint8_t opcode, padding;
        std::uint16_t recipient, sender, padding_word;
        Dword tick;
        std::uint8_t frames, tail[3];
    } packet;
    static_assert(sizeof(GhostPeriod) == 16 && offsetof(GhostPeriod, frames) == 12);
    packet.sender = packets::at<std::uint16_t>(client, 376);
    packet.frames = frames;
    packet.recipient = identifier;
    packet.opcode = 46;
    packet.tick = game::race_ticks;
    packets::send(packets::bytes(client) + 72, &packet, sizeof(packet), 4);
}
} // namespace nfs::network::snapshots
