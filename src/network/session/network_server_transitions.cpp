#include "network_lifecycle_runtime.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::network::session {
extern players::Visitor initial_car_visitor;

Dword all_players_in_phase(Dword *players, std::uint16_t phase) {
    alignas(4) Byte request[12];
    at<Dword>(request, 0) = 1;
    at<Dword>(request, 4) = 1;
    at<std::uint16_t>(request, 8) = phase;
    Dword *visitor = players ? reinterpret_cast<Dword *>(bytes(players) + 72) : nullptr;
    queue::visit(players, 0, reinterpret_cast<Dword>(request), visitor, 0, nullptr);
    return at<Dword>(request, 0);
}

void boot_route(void *route, Dword reason) {
    alignas(4) Byte packet[8];
    packet[0] = 16;
    at<Dword>(packet, 4) = reason;
    packets::send(route, packet, sizeof(packet));
    at<Dword>(route, 12) = 0;
}

Dword has_loading_player(const std::int16_t *identifier) {
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    for (int index = 0; index < static_cast<int>(queue::count(player_queue())); ++index)
        if (queue::copy_nth(player_queue(), 0, index, 0, reinterpret_cast<Dword>(&player), nullptr,
                            0, nullptr) &&
            (!identifier || at<std::int16_t>(&player, 2) == *identifier) && player.phase == 7)
            return 1;
    return 0;
}

void begin_server_load(void *owner) {
    void *rules = system(72);
    auto *signal = reinterpret_cast<kernel::Signal *>(bytes(owner) + 16);
    kernel::signal_wait(signal);
    if (at<int>(rules, 168) > 0) {
        at<std::uint16_t>(owner, 12) = 4;
        set_game_phase(network_game(), at<std::int16_t>(owner, 12));
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        for (int index = 0; index < static_cast<int>(queue::count(player_queue())); ++index)
            if (queue::copy_nth(player_queue(), 0, index, 0, reinterpret_cast<Dword>(&player),
                                nullptr, 0, nullptr)) {
                alignas(4) Byte packet[92];
                ui::initialize_car_settings(reinterpret_cast<ui::CarSettings *>(packet + 8));
                at<std::uint16_t>(packet, 6) = player.slot;
                at<std::uint16_t>(packet, 2) = std::uint16_t(player.identifier);
                for (unsigned word = 0; word < 21; ++word)
                    at<Dword>(packet, 8 + word * 4) = at<Dword>(&player.car, word * 4);
                at<std::int16_t>(packet, 4) = player.phase;
                packet[0] = 7;
                broadcast(static_cast<Dword *>(owner), packet, sizeof(packet), 32, nullptr);
            }
        services::pending::transition_network(network_game());
        kernel::signal_set(signal);
        queue::visit(player_queue(), 0, 0, reinterpret_cast<Dword *>(&initial_car_visitor), 0,
                     nullptr);
        if (at<Dword>(rules, 152) == 1) {
            int opponents = static_cast<int>(at<Dword>(rules, 168) - 1);
            if (opponents < 1)
                opponents = 1;
            packets::pending::set_game_rule(static_cast<Dword *>(rules), 7, Dword(opponents));
        }
        alignas(4) Byte packet[92];
        for (unsigned offset = 64; offset < 92; offset += 4)
            at<Dword>(packet, offset) = 0;
        packet[0] = 2;
        packets::export_server_settings(static_cast<Dword *>(owner), packet + 28);
        at<std::uint16_t>(packet, 26) = at<std::uint16_t>(owner, 12);
        std::strcpy(reinterpret_cast<char *>(packet + 1), reinterpret_cast<char *>(owner) + 172);
        broadcast(static_cast<Dword *>(owner), packet, sizeof(packet), 32, nullptr);
        at<Dword>(owner, 240) = transport_time() + 30000;
    } else {
        alignas(4) Byte packet[20];
        packet[0] = 48;
        at<Dword>(packet, 4) = 1671;
        at<Dword>(packet, 8) = 0;
        at<Dword>(packet, 12) = Dword(-2);
        at<Byte>(packet, 16) &= 252;
        broadcast(static_cast<Dword *>(owner), packet, sizeof(packet), 0, nullptr);
        at<Dword>(owner, 36) = 0;
        kernel::signal_set(signal);
    }
}

void schedule_server_start(void *owner) {
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    Dword maximum_delay = 0;
    for (int index = 0; index < static_cast<int>(queue::count(player_queue())); ++index)
        if (queue::copy_nth(player_queue(), 0, index, 0, reinterpret_cast<Dword>(&player), nullptr,
                            0, nullptr)) {
            const Dword delay = at<Dword>(&player, 132) >> 1;
            if (maximum_delay < delay)
                maximum_delay = delay;
        }
    if (maximum_delay > 10000)
        maximum_delay = 10000;
    at<Dword>(owner, 240) = transport_time() + maximum_delay;
    at<Dword>(owner, 236) = 1;
}

void finish_server_start(void *owner) {
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    alignas(4) Byte route[304];
    const Dword now = transport_time();
    bool waiting = false;
    auto send_settings = [&]() {
        Dword *server_owner = server();
        alignas(4) Byte packet[92];
        for (unsigned offset = 64; offset < 92; offset += 4)
            at<Dword>(packet, offset) = 0;
        packet[0] = 2;
        packets::export_server_settings(server_owner, packet + 28);
        at<std::uint16_t>(packet, 26) = at<std::uint16_t>(server_owner, 12);
        std::strcpy(reinterpret_cast<char *>(packet + 1),
                    reinterpret_cast<char *>(server_owner) + 172);
        packets::send(route, packet, sizeof(packet));
    };
    for (int index = 0; index < static_cast<int>(queue::count(player_queue())); ++index)
        if (queue::copy_nth(player_queue(), 0, index, 0, reinterpret_cast<Dword>(&player), nullptr,
                            0, nullptr) &&
            player.phase == 9 && at<std::int16_t>(&player, 118) == 4) {
            const Dword deadline = at<Dword>(owner, 240) - (at<Dword>(&player, 132) >> 1);
            if (static_cast<std::int32_t>(deadline - now) <= 0) {
                if (queue::copy_matching(route_queue(), reinterpret_cast<Dword>(bytes(&player) + 2),
                                         reinterpret_cast<Dword>(route), nullptr) &&
                    !at<Dword>(route, 4))
                    send_settings();
                at<std::int16_t>(&player, 118) = 5;
                queue::replace(player_queue(), reinterpret_cast<Dword>(&player),
                               reinterpret_cast<Dword>(&player), nullptr);
            } else
                waiting = true;
        }
    if (waiting)
        return;
    for (int index = 0; index < static_cast<int>(queue::count(player_queue())); ++index)
        if (queue::copy_nth(player_queue(), 0, index, 0, reinterpret_cast<Dword>(&player), nullptr,
                            0, nullptr) &&
            (player.phase != 9 || at<std::int16_t>(&player, 118) != 5) &&
            queue::copy_matching(route_queue(), reinterpret_cast<Dword>(bytes(&player) + 2),
                                 reinterpret_cast<Dword>(route), nullptr) &&
            !at<Dword>(route, 4))
            send_settings();
    at<Dword>(owner, 236) = at<Dword>(owner, 240) = 0;
    set_game_phase(network_game(), at<std::int16_t>(owner, 12));
    at<Dword>(owner, 28) = transport_time();
    at<Dword>(owner, 36) = 0;
}
} // namespace nfs::network::session
