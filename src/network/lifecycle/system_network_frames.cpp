#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/network/session/network_rules.hpp"
#include <mmsystem.h>
#include <cstring>
namespace nfs::lifecycle::table_entries {
using namespace game::runtime;
using namespace network::packets;

void __fastcall shutdown_server_state(System *object, void *) {
    auto *owner = reinterpret_cast<Dword *>(object);
    at<std::uint16_t>(owner, 12) = 1;
    at<Dword>(owner, 44) = 0;
    network::set_game_phase(network::network_game(), at<std::int16_t>(owner, 12));
    if (at<Dword>(owner, 48))
        network::graphics_enabled = network::saved_graphics_enabled;
}

void __fastcall unload_client_state(System *object, void *) {
    auto *owner = reinterpret_cast<Dword *>(object);
    const auto phase = at<std::uint16_t>(owner, 380);
    if (phase == 8 || phase == 9)
        at<std::uint16_t>(owner, 380) = 7;
    at<std::uint16_t>(owner, 378) = 65535;
    at<Dword>(owner, 60) = at<Dword>(owner, 2092) = at<Dword>(owner, 432) =
        at<Dword>(owner, 64) = 0;
    at<Byte>(owner, 2104) = 0;
    at<std::uint16_t>(owner, 2096) = 0;
    if (at<Dword>(owner, 84) && at<Dword>(system(96), 36) == 24) {
        alignas(4) Byte leave[6];
        leave[0] = 28;
        at<std::uint16_t>(leave, 2) = at<std::uint16_t>(owner, 376);
        at<std::uint16_t>(leave, 4) = 0;
        send(bytes(owner) + 72, leave, 6);
        const int track = static_cast<int>(rule_value(system(72), 5));
        if (track >= 0 && track < 12)
            network::packets::pending::select_track(system(40), track);
        alignas(4) Byte ping[8];
        ping[0] = 39;
        at<Dword>(ping, 4) = timeGetTime();
        at<Dword>(owner, 432) = timeGetTime();
        send(bytes(owner) + 72, ping, 8, 4);
        at<Dword>(network::transport, 28) = 0;
    }
    for (unsigned offset = 0; offset < 32; offset += 4)
        at<Dword>(owner, 2105 + offset) = 0;
    for (unsigned offset = 0; offset < 84; offset += 4)
        at<Dword>(owner, 436 + offset) = 0;
    for (unsigned offset = 0; offset < 1572; offset += 4)
        at<Dword>(owner, 520 + offset) = 0;
}
// Preserve unwritten header/padding and separate timeGetTime calls.
void __fastcall reset_server_round(System *object, void *) {
    auto *owner = reinterpret_cast<Dword *>(object);
    if (!at<Dword>(owner, 44))
        return;
    at<std::uint16_t>(owner, 12) = 3;
    network::set_game_phase(network::network_game(), at<std::int16_t>(owner, 12));
    at<Dword>(owner, 32) = timeGetTime();
    at<Dword>(owner, 28) = timeGetTime();
    at<Dword>(owner, 36) = at<Dword>(owner, 24) = 0;
    alignas(4) Byte packet[92];
    packet[0] = 2;
    std::memset(packet + 64, 0, 28);
    export_server_settings(owner, packet + 28);
    at<std::uint16_t>(packet, 26) = at<std::uint16_t>(owner, 12);
    std::strcpy(reinterpret_cast<char *>(packet + 1), reinterpret_cast<char *>(owner) + 172);
    network::broadcast(owner, packet, sizeof(packet), 32, nullptr);
    network::ui::PlayerRecord player;
    std::memset(bytes(&player) + 32, 0, 20);
    at<Dword>(&player, 52) = 4294967295u;
    at<Byte>(&player, 56) = 0;
    at<Dword>(&player, 88) = 1065353216;
    std::memset(bytes(&player) + 92, 0, 24);
    for (Dword index = 0; index < queue::count(player_queue()); ++index) {
        if (!queue::copy_nth(player_queue(), 0, index, 0, address_of(&player), nullptr, 0, nullptr))
            continue;
        alignas(4) Byte route[304];
        if (queue::copy_matching(route_queue(owner), address_of(bytes(&player) + 2),
                                 address_of(route), nullptr)) {
            at<Dword>(route, 20) = timeGetTime();
            queue::replace(route_queue(owner), address_of(bytes(&player) + 2), address_of(route),
                           nullptr);
        }
    }
}
} // namespace nfs::lifecycle::table_entries
