#include "nfs/network/packets/network_packet_handlers.hpp"
#include <winsock.h>
#include <cstring>
namespace nfs::network::packets {
namespace {
void remove_vote(void *record, void *key) {
    auto *queue_owner = static_cast<Dword *>(record);
    auto *comparer = record ? reinterpret_cast<Dword *>(bytes(record) + 68) : nullptr;
    queue::erase(queue_owner, address_of(key), comparer);
    queue::append(queue_owner, address_of(key), address_of("listnode"));
}
void vote_message(Dword *owner, Dword reference, std::int16_t identifier) {
    alignas(4) Byte packet[20];
    packet[0] = 48;
    at<Dword>(packet, 4) = reference;
    at<int>(packet, 8) = identifier;
    at<Dword>(packet, 12) = 4294967294u;
    at<Dword>(packet, 16) |= 3;
    broadcast(owner, packet, sizeof(packet), 0, nullptr);
}
void tally(Dword *owner, void *descriptor) {
    auto *votes = at<Dword *>(owner, 312);
    at<Dword>(votes, 76) = 0;
    queue::visit(votes, 0, address_of(descriptor), votes + 72 / 4, 0, nullptr);
}
} // namespace

void dispatch_vote(Dword *owner, void *packet) {
    const Byte kind = at<Byte>(packet, 6);
    auto *key = bytes(packet) + 4;
    auto *votes = at<Dword *>(owner, 312);
    if (kind == 1) {
        ScopedString text;
        gui::string_copy(&text.value, &lifecycle::construction::unresolved::shared_string);
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        std::memset(&player, 0, sizeof(player));
        if (copy_player(key, &player)) {
            if (player.phase == 9 || at<std::int16_t>(owner, 12) != 5 ||
                static_cast<int>(players::selected_players(player_queue())) < 2)
                remove_vote(votes, key);
            vote_message(owner, player.phase == 9 ? 1629 : 1630,
                         static_cast<std::int16_t>(player.identifier));
        }
        if (at<std::int16_t>(network_game(), 8) > 4) {
            at<Dword>(votes, 76) = 0;
            queue::visit(votes, 0, address_of(&kind), nullptr, 0, nullptr);
            if (players::selected_players(player_queue()) <= at<Dword>(votes, 76))
                at<Dword>(owner, 156) = 1;
        }
        return;
    }
    if (kind != 2 && kind != 3) {
        remove_vote(votes, key);
        return;
    }
    if (!at<Dword>(owner, 140))
        return;
    ScopedString text;
    gui::string_copy(&text.value, &lifecycle::construction::unresolved::shared_string);
    ui::PlayerRecord player{}, target{};
    ui::initialize_car_settings(&player.car);
    if (kind == 2) {
        ui::initialize_car_settings(&target.car);
        std::memset(&player, 0, sizeof(player));
        std::memset(&target, 0, sizeof(target));
        if (copy_player(key, &player) && copy_player(bytes(packet) + 8, &target)) {
            if (player.phase == 9 || at<std::int16_t>(owner, 12) != 5)
                remove_vote(votes, key);
            vote_message(owner, 1631, static_cast<std::int16_t>(target.identifier));
        }
        alignas(4) Byte descriptor[8];
        at<std::uint16_t>(descriptor, 0) = 0;
        descriptor[2] = kind;
        at<Dword>(descriptor, 4) = at<Dword>(packet, 8);
        tally(owner, descriptor);
        const int total = at<int>(votes, 76);
        const int electorate = at<std::int16_t>(owner, 12) == 5
                                   ? players::selected_players(player_queue())
                                   : queue::count(player_queue());
        Dword route[76];
        if (total > 1 && electorate / 2 + 1 <= total &&
            queue::copy_matching(route_queue(owner), address_of(bytes(&target) + 2),
                                 address_of(route), nullptr)) {
            Dword reference;
            if (!route[1]) {
                const int identifier = static_cast<std::int16_t>(target.identifier);
                auto *authentication = pending::authentication_record(identifier);
                if (authentication && at<Byte>(authentication, 325)) {
                    in_addr ip{};
                    Dword copy[76];
                    if (find_player_route(owner, identifier, copy) && copy[9]) {
                        void *endpoint = network::address(reinterpret_cast<void *>(copy[9]));
                        if (endpoint)
                            ip.s_addr = at<Dword>(endpoint, 4);
                    }
                    const auto *key_text = reinterpret_cast<char *>(authentication) + 325;
                    if (!pending::is_booted(key_text))
                        pending::record_boot(key_text, target.name, inet_ntoa(ip));
                }
                while (remove_server_client(owner, nullptr, route[0])) {
                }
                alignas(4) Byte rejection[8];
                rejection[0] = 16;
                at<Dword>(rejection, 4) = 14;
                send(route, rejection, sizeof(rejection));
                route[3] = 0;
                reference = 1633;
            } else
                reference = 1632;
            vote_message(owner, reference, static_cast<std::int16_t>(target.identifier));
        }
        return;
    }
    std::memset(&player, 0, sizeof(player));
    const Dword choice = at<Dword>(packet, 8);
    ScopedString track;
    gui::string_reference(&track.value, track_reference(choice));
    gui::string_make_narrow(&track.value);
    if (copy_player(key, &player)) {
        if (player.phase == 9 || at<std::int16_t>(owner, 12) != 5)
            remove_vote(votes, key);
        if (choice == 12)
            vote_message(owner, 1635, static_cast<std::int16_t>(player.identifier));
        else {
            gui::string_format_ascii(&text.value, "%s voted for the %s track", player.name,
                                     gui::string_data(&track.value));
            broadcast_chat(owner, gui::string_ascii(&text.value), nullptr);
        }
    }
    alignas(4) Byte descriptor[8];
    at<std::uint16_t>(descriptor, 0) = 0;
    descriptor[2] = kind;
    at<Dword>(descriptor, 4) = choice;
    tally(owner, descriptor);
    const int total = at<int>(votes, 76);
    const int needed = at<std::int16_t>(owner, 12) == 5 ? players::selected_players(player_queue())
                                                         : queue::count(player_queue()) / 2 + 1;
    if (needed <= total) {
        at<Dword>(owner, 152) = choice;
        at<Dword>(owner, 148) = 1;
    }
}
} // namespace nfs::network::packets
