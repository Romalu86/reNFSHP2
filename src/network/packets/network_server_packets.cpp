#include "nfs/network/packets/network_packet_handlers.hpp"
#include "network_packet_x87.hpp"
#include <cstdlib>
#include <cstring>
#include <mmsystem.h>
namespace nfs::network {
using namespace packets;
namespace {
void server_settings(Dword *owner, void *route) {
    alignas(4) Byte packet[92];
    clear_server_settings(packet + 28);
    packet[0] = 2;
    export_server_settings(owner, packet + 28);
    at<std::uint16_t>(packet, 26) = at<std::uint16_t>(owner, 12);
    std::strcpy(reinterpret_cast<char *>(packet + 1), reinterpret_cast<char *>(owner) + 172);
    send(route, packet, sizeof(packet));
}
void visit_routes(Dword *owner, void *packet, Dword size, Dword flags, void *exclude) {
    struct Descriptor {
        std::uint16_t kind, padding;
        void *packet;
        Dword size, flags;
        void *exclude;
    } descriptor;
    descriptor.kind = 1;
    descriptor.packet = packet;
    descriptor.size = size;
    descriptor.flags = flags;
    descriptor.exclude = exclude;
    queue::visit(route_queue(owner), 0, address_of(&descriptor), visitor(owner), 0, nullptr);
}
void join_player(Dword *owner, Dword *route, Byte *packet) {
    ScopedString name;
    gui::string_assign(&name.value, reinterpret_cast<char *>(packet + 8));
    if (!gui::string_data(&name.value))
        gui::string_reference(&name.value, 2616);
    gui::string_make_narrow(&name.value);
    Dword suffix = 0;
    while (
        queue::copy_matching(player_queue(), address_of(gui::string_ascii(&name.value)), 0,
                             reinterpret_cast<Dword *>(&packets::pending::name_comparer))) {
        ScopedString changed;
        gui::string_format_ascii(&changed.value, "(%d)%s", suffix,
                                 reinterpret_cast<char *>(packet + 8));
        gui::string_substring(&changed.value, 0, 24, &name.value);
        ++suffix;
    }
    gui::string_make_narrow(&name.value);
    alignas(4) Byte joined[30];
    joined[0] = 3;
    at<std::uint16_t>(joined, 2) = at<std::uint16_t>(owner, 224)++;
    at<std::uint16_t>(joined, 4) = at<std::uint16_t>(route, 0);
    std::strncpy(reinterpret_cast<char *>(joined + 6), gui::string_ascii(&name.value), 24);
    joined[29] = 0;
    add_player(player_queue(), at<std::uint16_t>(joined, 2), reinterpret_cast<char *>(joined + 6),
               at<std::uint16_t>(route, 0));
    broadcast(owner, joined, sizeof(joined), 0, nullptr);
    if (route[1])
        return;
    queue::visit(player_queue(), 0, address_of(route), nullptr, 0, nullptr);
    if (at<std::int16_t>(network_game(), 8) > 2) {
        ui::CarSettings temporary;
        ui::initialize_car_settings(&temporary);
        alignas(4) Byte state[96];
        state[0] = 47;
        for (unsigned slot = 0; slot < 24; ++slot) {
            if (vehicle(slot) && (player_left(race(), slot) & 255)) {
                at<Dword>(state, 4) = slot;
                std::memcpy(state + 8, bytes(system(72)) + 248 + slot * 84, 84);
                state[92] = 0;
                send(route, state, sizeof(state));
            }
        }
    }
    server_settings(owner, route);
}
} // namespace
// Route admission and every saved opcode arm are retained.
void __fastcall dispatch_server_packet(Dword *owner, void *, Dword route_address, Dword packet_address,
                                       Dword size) {
    auto *route = reinterpret_cast<Dword *>(route_address);
    auto *packet = reinterpret_cast<Byte *>(packet_address);
    if (!route[3])
        return;
    switch (packet[0]) {
    case 18: {
        if (route[1]) {
            join_player(owner, route, packet);
            return;
        }
        Dword reason;
        if (static_cast<int>(queue::count(player_queue())) >= at<Byte>(owner, 96))
            reason = 19;
        else if (at<Dword>(packet, 4) != 18088178)
            reason = 1;
        else if (at<Byte>(owner, 196) && std::strcmp(reinterpret_cast<char *>(packet + 32),
                                                      reinterpret_cast<char *>(owner) + 196))
            reason = 2;
        else {
            reason = 0;
            if (packets::pending::validate_key(reinterpret_cast<char *>(packet + 56),
                                                        &reason, nullptr)) {
                if (at<std::int16_t>(owner, 12) != 1) {
                    join_player(owner, route, packet);
                    return;
                }
                reason = 3;
            }
        }
        alignas(4) Byte rejection[8];
        rejection[0] = 16;
        at<Dword>(rejection, 4) = reason;
        send(route, rejection, sizeof(rejection));
        route[3] = 0;
        return;
    }
    case 22: {
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        const bool found = copy_player(packet + 2, &player);
        auto *signal = reinterpret_cast<kernel::Signal *>(bytes(owner) + 16);
        const bool locked = kernel::signal_ready(signal);
        const int selected_class = static_cast<int>(rule_value(system(72), 15));
        const int requested_class = vehicle_class(system(52) + 1, at<int>(packet, 64));
        const bool allowed =
            (selected_class == requested_class || selected_class == 6) && at<int>(packet, 64) != 7;
        if (!locked || at<std::int16_t>(owner, 12) != 3 || !found || !allowed ||
            !vehicle_resource(system(52) + 1, at<int>(packet, 64))) {
            Dword copy[76];
            std::memcpy(copy, route, sizeof(copy));
            copy[2] = 0;
            Byte rejection = 11;
            send(copy, &rejection, 1);
        } else {
            ui::CarSettings car;
            std::memcpy(&car, packet + 4, sizeof(car));
            ui::CarSettings unused;
            ui::initialize_car_settings(&unused);
            alignas(4) Byte state[92];
            auto slot = player.slot;
            if (slot == -1) {
                slot = static_cast<std::int16_t>(at<Dword>(system(72), 168));
                at<int>(system(72), 168) = slot + 1;
                alignas(4) Byte message[20];
                message[0] = 48;
                at<Dword>(message, 4) = 1638;
                at<Dword>(message, 8) = at<Dword>(system(72), 168);
                at<Dword>(message, 12) = 4294967294u;
                at<volatile Byte>(message, 16) =
                    Byte((at<volatile Byte>(message, 16) | 1u) & ~2u);
                broadcast(owner, message, sizeof(message), 0, nullptr);
            }
            if (slot < 8)
                at<int>(&car, 20) = 7 - slot;
            state[0] = 7;
            at<std::uint16_t>(state, 2) = at<std::uint16_t>(packet, 2);
            at<std::int16_t>(state, 4) = 9;
            at<std::int16_t>(state, 6) = slot;
            std::memcpy(state + 8, &car, sizeof(car));
            visit_routes(owner, state, sizeof(state), 0, nullptr);
            players::apply_state(player_queue(), state);
        }
        if (locked)
            kernel::signal_set(signal);
        return;
    }
    case 23:
        release_player_slot(owner, at<std::uint16_t>(packet, 2));
        return;
    case 24:
        if (size < 141) {
            if (!at<Dword>(client(), 68) && at<Dword>(network_game(), 92)) {
                ScopedString text;
                gui::string_assign(&text.value, reinterpret_cast<char *>(packet + 8));
                notice(&text.value, at<std::int16_t>(packet, 2));
            }
            packet[0] = 13;
            visit_routes(owner, packet, size, 0, route);
        }
        return;
    case 28: {
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        if (copy_player(packet + 2, &player)) {
            const auto phase = at<std::int16_t>(packet, 4);
            if (phase == 0 && (player.phase == 9 || player.phase == 8))
                player.phase = 7;
            else if (player.phase == 7) {
                player.phase = 8;
                if (at<std::int16_t>(owner, 12) > 4)
                    packets::pending::send_race_players(owner, route);
            }
            at<std::int16_t>(&player, 118) = phase;
            replace_player(packet + 2, &player);
            if (phase != at<std::int16_t>(network_game(), 8))
                server_settings(owner, route);
        }
        return;
    }
    case 29: {
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        ScopedString text;
        if (queue::copy_matching(
                player_queue(), address_of(packet + 4), address_of(&player),
                reinterpret_cast<Dword *>(&packets::pending::name_comparer)))
            return;
        if (copy_player(packet + 2, &player) &&
            std::strcmp(player.name, reinterpret_cast<char *>(packet + 4))) {
            ScopedString before, after, pattern;
            gui::string_assign(&before.value, player.name);
            gui::string_assign(&after.value, reinterpret_cast<char *>(packet + 4));
            gui::string_make_wide(&before.value);
            gui::string_make_wide(&after.value);
            gui::string_reference(&pattern.value, 1350);
            gui::string_format(&text.value, &pattern.value, gui::string_data(&before.value),
                               gui::string_data(&after.value));
            std::strncpy(player.name, reinterpret_cast<char *>(packet + 4), 24);
            player.name[23] = 0;
            replace_player(packet + 2, &player);
            notice(&text.value);
            visit_routes(owner, packet, size, 0, nullptr);
        }
        return;
    }
    case 30:
        packets::pending::receive_authentication(at<std::int16_t>(packet, 2), packet + 4);
        return;
    case 32:
        at<Dword>(owner, 28) = timeGetTime();
        visit_routes(owner, packet, size, 32, route);
        if (!route[1]) {
            kernel::signal_wait(reinterpret_cast<kernel::Signal *>(bytes(race()) + 156));
            Dword *race_state = race();
            const int slot = at<int>(packet, 4);
            using Method = void(__thiscall *)(Dword *, Dword, Dword, Dword);
            reinterpret_cast<const Method *>(race_state[0])[120 / 4](
                race_state, slot, at<Dword>(packet, 20), at<Dword>(packet, 8));
            kernel::signal_set(reinterpret_cast<kernel::Signal *>(bytes(race()) + 156));
            if (auto *car = vehicle(slot))
                packet_native::apply_car_speeds(car, packet + 12);
        }
        return;
    case 33: {
        if (!at<Dword>(client(), 68) && at<Dword>(network_game(), 92)) {
            ScopedString text;
            gui::string_assign(&text.value, reinterpret_cast<char *>(packet + 18));
            notice(&text.value, at<std::int16_t>(packet, 2));
        }
        unsigned count = packet[4];
        if (!count)
            return;
        if (count > 6)
            count = 6;
        unsigned length =
            static_cast<unsigned>(std::strlen(reinterpret_cast<char *>(packet + 18)));
        if (static_cast<int>(length) > 512)
            length = 512;
        const unsigned allocation = length + 12;
        auto *chat = static_cast<Byte *>(
            files::archives::allocate_entry(allocation, "ChatMsg", gui::application_pool));
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        chat[0] = 13;
        std::strncpy(reinterpret_cast<char *>(chat + 8), reinterpret_cast<char *>(packet + 18),
                     length);
        at<std::uint16_t>(chat, 2) = at<std::uint16_t>(packet, 2);
        at<Dword>(chat, 4) = 1;
        chat[length + 8] = 0;
        Dword copy[76];
        for (unsigned index = count; index; --index)
            if (copy_player(packet + 6 + (index - 1) * 2, &player) &&
                queue::copy_matching(route_queue(owner), address_of(bytes(&player) + 2),
                                     address_of(copy), nullptr))
                send(copy, chat, allocation);
        gui::application_release(chat);
        return;
    }
    case 34:
    case 44: {
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        if (copy_player(packet + 2, &player)) {
            if (packet[0] == 34)
                at<Byte>(&player, 120) = packet[4];
            else
                at<Dword>(&player, 132) = at<Dword>(packet, 4);
            replace_player(packet + 2, &player);
            visit_routes(owner, packet, size, 32, route);
        }
        return;
    }
    case 35:
        packets::pending::receive_client_car(owner, route, packet);
        return;
    case 37:
        if (!(bits::server_decoders & 1)) {
            bits::server_decoders |= 1;
            bits::construct(&bits::server_car_stream);
            std::atexit(bits::finalize_server_car);
        }
        bits::reset(&bits::server_car_stream);
        bits::bind(&bits::server_car_stream, packet, size);
        bits::decode_client_car(&bits::server_car_stream, bits::server_car);
        packets::pending::receive_client_car(owner, route, bits::server_car);
        return;
    case 39: {
        alignas(4) Byte reply[8];
        reply[0] = 40;
        at<Dword>(reply, 4) = at<Dword>(packet, 4);
        send(route, reply, sizeof(reply), 4);
        return;
    }
    case 43:
        dispatch_vote(owner, packet);
        return;
    case 46: {
        alignas(4) Byte reply[12];
        reply[0] = 45;
        at<Dword>(reply, 4) = at<Dword>(packet, 8);
        reply[8] = packet[12];
        ui::PlayerRecord first, second;
        ui::initialize_car_settings(&first.car);
        ui::initialize_car_settings(&second.car);
        Dword copy[76];
        if (copy_player(packet + 2, &first) && copy_player(packet + 4, &second) &&
            queue::copy_matching(route_queue(owner), address_of(bytes(&first) + 2),
                                 address_of(copy), nullptr))
            send(copy, reply, sizeof(reply), 4);
        return;
    }
    case 50:
        if (size <= 268) {
            if (!at<Dword>(client(), 68)) {
                ScopedString text;
                gui::string_assign_wide(&text.value, reinterpret_cast<wchar_t *>(packet + 8));
                notice(&text.value, at<std::int16_t>(packet, 2));
            }
            packet[0] = 49;
            visit_routes(owner, packet, size, 0, route);
        }
        return;
    }
}
} // namespace nfs::network
