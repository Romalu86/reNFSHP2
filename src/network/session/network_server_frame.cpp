#include "network_lifecycle_runtime.hpp"
#include "server_runtime.hpp"
namespace nfs::network::session {
extern players::Visitor timeout_visitor;
// Shared formatted-text packet layout.
struct RaceNotice {
    Byte opcode, padding[3];
    Dword reference, argument, sender;
    Byte flags, tail[3];
};
static_assert(sizeof(RaceNotice) == 20);
} // namespace nfs::network::session
namespace nfs::services::pending {

void tick_server(void *owner) {
    using namespace network::session;
    namespace queue = network::session::queue;
    namespace p = network::packets;
    if (at<Dword>(owner, 232))
        return;
    at<Dword>(owner, 232) = 1;
    if (p::pending::session_kind != 2) {
        update_server_query();
        update_authentication();
    }
    if (at<Dword>(owner, 104))
        queue::visit(network::server_queue(static_cast<Dword *>(owner)), 0,
                     reinterpret_cast<Dword>(owner), reinterpret_cast<Dword *>(&timeout_visitor), 0,
                     nullptr);
    RaceNotice notice;
    auto text = [&](Dword reference, Dword sender, Dword argument, Byte flags, bool through_visitor) {
        notice.opcode = 48;
        notice.reference = reference;
        notice.argument = argument;
        notice.sender = sender;
        notice.flags = Byte((notice.flags & 252) | (flags & 3));
        if (!through_visitor)
            network::broadcast(static_cast<Dword *>(owner), &notice, sizeof(notice), 0, nullptr);
        else {
            struct Descriptor {
                std::uint16_t kind, padding;
                void *data;
                Dword size, flags, exclude;
            } descriptor;
            descriptor.kind = 1;
            descriptor.data = &notice;
            descriptor.size = 20;
            descriptor.flags = 0;
            descriptor.exclude = 0;
            queue::visit(network::server_queue(static_cast<Dword *>(owner)), 0,
                         reinterpret_cast<Dword>(&descriptor),
                         p::visitor(static_cast<Dword *>(owner)), 0, nullptr);
        }
    };
    auto finish = [&]() { at<Dword>(owner, 232) = 0; };
    if (at<Dword>(owner, 148)) {
        at<Dword>(owner, 64) = at<Dword>(owner, 152);
        text(1637, Dword(-2), 0, 0, false);
        at<Dword>(owner, 148) = at<Dword>(owner, 152) = 0;
        application::pending::rotate_server_race(owner);
        finish();
        return;
    }
    network::ui::PlayerRecord player;
    network::ui::initialize_car_settings(&player.car);
    alignas(4) Byte route[304];
    for (int index = 0; index < static_cast<int>(queue::count(player_queue())); ++index) {
        if (queue::copy_nth(player_queue(), 0, index, 0, reinterpret_cast<Dword>(&player), nullptr,
                            0, nullptr) &&
            player.phase != 7 &&
            queue::copy_matching(network::server_queue(static_cast<Dword *>(owner)),
                                 reinterpret_cast<Dword>(bytes(&player) + 2),
                                 reinterpret_cast<Dword>(route), nullptr) &&
            !at<Dword>(route, 16)) {
            network::packets::broadcast_chat(
                static_cast<Dword *>(owner),
                reinterpret_cast<char *>(services::pending::server_settings) + 704, route);
            at<Dword>(route, 16) = 1;
            queue::replace(network::server_queue(static_cast<Dword *>(owner)),
                           reinterpret_cast<Dword>(bytes(&player) + 2),
                           reinterpret_cast<Dword>(route), nullptr);
        }
    }
    switch (at<std::int16_t>(owner, 12)) {
    case 3: {
        if (at<Dword>(owner, 228))
            at<Dword>(owner, 228) = 0;
        if (!at<Dword>(owner, 108))
            break;
        bool elapsed = false;
        const Dword now = network::transport_time();
        if (at<Dword>(owner, 32) && at<Dword>(owner, 160) &&
            at<Dword>(owner, 160) < now - at<Dword>(owner, 32))
            elapsed = true;
        const Dword start = at<Dword>(owner, 36), duration = at<Dword>(owner, 164);
        bool countdown_finished = false;
        if (start && duration) {
            const Dword spent = now - start;
            if (spent > duration)
                countdown_finished = true;
            else {
                const Dword selected = network::players::selected_players(player_queue());
                if (queue::count(player_queue()) != selected ||
                    static_cast<int>(queue::count(player_queue())) < 2) {
                    const Dword seconds = (duration - spent) / 1000;
                    if (seconds < 31 && seconds && at<Dword>(owner, 40) != seconds) {
                        at<Dword>(owner, 40) = seconds;
                        text(seconds == 1 ? 1639 : 1640, Dword(-3), seconds == 1 ? 0 : seconds,
                             seconds == 1 ? 0 : 1, true);
                    }
                } else {
                    text(1654, Dword(-3), 0, 0, true);
                    countdown_finished = true;
                }
            }
        }
        if (countdown_finished || elapsed) {
            text(1617, Dword(-3), 0, 0, false);
            text(1672, Dword(-2), 0, 0, false);
            begin_server_load(owner);
        }
        if (at<Dword>(owner, 168) && !at<Dword>(owner, 36) && at<int>(system(72), 168) > 0 &&
            !has_loading_player(nullptr)) {
            finish();
            at<Dword>(owner, 40) = at<Dword>(owner, 164) / 1000;
            at<Dword>(owner, 36) = now;
            return;
        }
        break;
    }
    case 4:
        if (all_players_in_phase(player_queue(), 4)) {
            at<std::uint16_t>(owner, 12) = 5;
            at<Dword>(owner, 28) = network::transport_time();
            text(1673, Dword(-2), 0, 0, true);
            schedule_server_start(owner);
            finish();
            return;
        }
        if (at<Dword>(owner, 240) && at<Dword>(owner, 240) < network::transport_time()) {
            network::ui::initialize_car_settings(&player.car);
            for (int index = 0; index < static_cast<int>(queue::count(player_queue())); ++index)
                if (queue::copy_nth(player_queue(), 0, index, 0, reinterpret_cast<Dword>(&player),
                                    nullptr, 0, nullptr) &&
                    player.phase == 9 && at<std::int16_t>(&player, 118) != 4 &&
                    network::packets::find_player_route(
                        static_cast<Dword *>(owner), Dword(std::int16_t(player.identifier)), route)) {
                    while (network::remove_server_client(static_cast<Dword *>(owner), nullptr,
                                                         at<Dword>(route, 0))) {
                    }
                    boot_route(route, 7);
                }
        }
        break;
    case 5:
        if (at<Dword>(owner, 236)) {
            finish_server_start(owner);
            finish();
            return;
        }
        if (!network::players::selected_players(player_queue())) {
            text(1675, Dword(-2), 0, 0, true);
            at<Dword>(owner, 36) = 0;
            application::pending::rotate_server_race(owner);
            finish();
            return;
        }
        if (at<Dword>(owner, 156)) {
            void *vote = at<void *>(owner, 312);
            at<Dword>(vote, 76) = 0;
            alignas(4) Byte request[4];
            request[0] = 1;
            queue::visit(static_cast<Dword *>(vote), 0, reinterpret_cast<Dword>(request), nullptr, 0,
                         nullptr);
            if (at<int>(vote, 76) > 1)
                text(1804, Dword(-2), 0, 0, true);
            application::pending::rotate_server_race(owner);
            at<Dword>(owner, 156) = 0;
            finish();
            return;
        }
        if (at<Dword>(owner, 132) && at<Dword>(owner, 36) && at<Dword>(system(72), 152) != 1) {
            const Dword deadline = at<Dword>(owner, 36), now = network::transport_time();
            if (deadline < now) {
                text(1676, Dword(-2), 0, 0, true);
                method(at<void *>(system(92), 4), 2);
            } else {
                const Dword seconds = (deadline - now) / 1000;
                if ((seconds == 30 || seconds == 60 || seconds == 90) &&
                    at<Dword>(owner, 40) != seconds) {
                    at<Dword>(owner, 40) = seconds;
                    text(1819, Dword(-3), seconds, 1, true);
                }
            }
        }
        if (at<Dword>(owner, 128) &&
            at<Dword>(owner, 28) + at<Dword>(owner, 128) < network::transport_time()) {
            text(1677, Dword(-2), 0, 0, true);
            method(at<void *>(system(92), 4), 2);
        }
        update_server_cars(owner);
        finish();
        return;
    case 6:
        if (at<Dword>(owner, 112) < network::transport_time() - at<Dword>(owner, 36))
            application::pending::rotate_server_race(owner);
        break;
    }
    finish();
}
} // namespace nfs::services::pending
