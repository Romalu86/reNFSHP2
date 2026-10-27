#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../session/race_session.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::pending {
using network::players::Visitor;
void rotate_server_race(void *);
namespace {
Visitor finish_player_visitor{};
struct TextPacket {
    Byte opcode, padding[3];
    Dword reference, sender_target, sender;
    Byte reliable : 1;
    Byte local : 1;
    Byte reserved : 6;
    Byte tail[3];
};
static_assert(sizeof(TextPacket) == 20);
} // namespace

void send_race_text(void *server, Dword reference, Dword sender, Dword route, Dword reliable,
                    Dword recipient, Dword local) {
    TextPacket packet;
    packet.opcode = 48;
    packet.reference = reference;
    packet.sender_target = recipient;
    packet.sender = sender;
    packet.reliable = reliable != 0;
    packet.local = local != 0;
    if (route)
        network::send_packet(route, reinterpret_cast<Dword>(&packet), sizeof(packet), 0);
    else
        network::broadcast(static_cast<Dword *>(server), &packet, sizeof(packet), 0, nullptr);
}

Dword __fastcall record_network_player_result(Visitor *, void *, network::ui::PlayerRecord *player,
                                             Dword, Dword, Dword server) {
    if (player->phase == 9 && player->slot >= 0) {
        void *snapshot = at<void *>(system(92), 8);
        const int position = at<int>(snapshot, 34296 + Dword(player->slot) * 4);
        const unsigned base = at<Dword>(snapshot, 34620) == 1 ? 126 : 122;
        if (position == 0)
            ++at<std::uint16_t>(player, base);
        ++at<std::uint16_t>(player, base + 2);
        struct PlayerResultPacket {
            Byte opcode, padding;
            std::uint16_t identifier;
            Dword race_counts, series_counts;
        } packet;
        packet.opcode = 15;
        packet.race_counts = at<Dword>(player, 122);
        packet.series_counts = at<Dword>(player, 126);
        if (server) {
            packet.identifier = std::uint16_t(player->identifier);
            network::broadcast(reinterpret_cast<Dword *>(server), &packet, sizeof(packet), 32,
                               nullptr);
            if (position == 0)
                send_race_text(reinterpret_cast<void *>(server), 1628, Dword(-2), 0, 1,
                               Dword(std::int16_t(packet.identifier)), 1);
        }
    }
    return 1;
}

void destroy_finish_player_visitor(Visitor *visitor) {
    visitor->methods = lifecycle::construction::network::pending::visitor_interface_methods;
}

void *__fastcall delete_finish_player_visitor(Visitor *visitor, void *, Dword flags) {
    destroy_finish_player_visitor(visitor);
    if (flags & 1)
        gui::application_release(visitor);
    return visitor;
}

const Dword finish_player_methods[] = {reinterpret_cast<Dword>(record_network_player_result),
                                      reinterpret_cast<Dword>(delete_finish_player_visitor)};

Visitor *construct_finish_player_visitor(Visitor *visitor) {
    visitor->methods = lifecycle::construction::network::pending::visitor_interface_methods;
    visitor->methods = finish_player_methods;
    return visitor;
}

void finalize_finish_player_visitor() {
    destroy_finish_player_visitor(&finish_player_visitor);
}

void initialize_finish_player_visitor() {
    construct_finish_player_visitor(&finish_player_visitor);
    std::atexit(finalize_finish_player_visitor);
}

void finish_network_race(void *server) {
    if (!at<Dword>(server, 112)) {
        at<Dword>(server, 36) = 0;
        rotate_server_race(server);
        return;
    }
    if (at<std::int16_t>(server, 12) == 6)
        return;
    namespace q = lifecycle::construction::network::queue;
    q::visit(network::server_queue(system(104)), 0, reinterpret_cast<Dword>(server),
             reinterpret_cast<Dword *>(&finish_player_visitor), 0, nullptr);
    at<std::int16_t>(server, 12) = 6;
    network::set_game_phase(system(104), at<std::int16_t>(server, 12));
    struct StatePacket {
        Byte opcode;
        char name[25];
        std::int16_t phase;
        Byte settings[64];
    } packet;
    static_assert(sizeof(StatePacket) == 92 && offsetof(StatePacket, settings) == 28);
    for (unsigned offset = 36; offset < 64; offset += 4)
        at<Dword>(packet.settings, offset) = 0;
    packet.opcode = 2;
    network::packets::export_server_settings(static_cast<Dword *>(server), packet.settings);
    packet.phase = at<std::int16_t>(server, 12);
    std::strcpy(packet.name, reinterpret_cast<const char *>(bytes(server) + 172));
    network::broadcast(static_cast<Dword *>(server), &packet, sizeof(packet), 32, nullptr);
    at<Dword>(server, 36) = network::transport_time();
}
} // namespace nfs::application::pending
 
NFS_KERNEL_INITIALIZER(startup_initialize_finish_player_visitor, ".CRT$XCU_NFS_00690294",
                       nfs::application::pending::initialize_finish_player_visitor);
