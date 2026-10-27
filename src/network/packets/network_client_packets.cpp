#include "nfs/network/packets/network_packet_handlers.hpp"
#include "network_packet_x87.hpp"
#include "nfs/core/frame/game_step.hpp"
#include <cstdlib>
#include <cstring>
#include <mmsystem.h>
namespace nfs::network {
using namespace packets;
namespace {
void reference_notice(Dword reference) {
    ScopedString text;
    gui::string_reference(&text.value, reference);
    notice(&text.value);
}
void ping(Dword *owner) {
    alignas(4) Byte packet[8];
    packet[0] = 39;
    at<Dword>(packet, 4) = timeGetTime();
    at<Dword>(owner, 432) = timeGetTime();
    send(bytes(owner) + 72, packet, 8, 4);
}
} // namespace
// All switch arms from the saved 51-entry opcode map.
void __fastcall dispatch_client_packet(Dword *owner, void *, Dword route_address, Dword packet_address,
                                       Dword size) {
    auto *packet = reinterpret_cast<Byte *>(packet_address);
    auto *route = reinterpret_cast<Dword *>(route_address);
    switch (packet[0]) {
    case 2: {
        import_server_settings(owner, packet + 28);
        if (at<std::int16_t>(owner, 380) == 4 || at<std::int16_t>(owner, 380) == 6)
            at<std::int16_t>(owner, 380) = 7;
        set_game_phase(network_game(), at<std::int16_t>(packet, 26));
        Dword *game_state = network_game();
        at<std::uint16_t>(game_state, 10) = at<std::uint16_t>(game_state, 8);
        set_server_name(game_state, reinterpret_cast<char *>(packet + 1));
        return;
    }
    case 3:
        add_player(player_queue(), at<std::uint16_t>(packet, 2),
                   reinterpret_cast<char *>(packet + 6), at<std::uint16_t>(packet, 4));
        if (at<std::uint16_t>(packet, 4) == at<std::uint16_t>(owner, 72)) {
            at<std::uint16_t>(owner, 376) = at<std::uint16_t>(packet, 2);
            at<std::int16_t>(owner, 378) = -1;
            if (!route[1]) {
                at<std::int16_t>(owner, 380) = 6;
                ping(owner);
            } else
                at<std::int16_t>(owner, 380) = 7;
        }
        return;
    case 4:
        queue::erase(player_queue(), address_of(packet + 4), nullptr);
        queue::append(player_queue(), address_of(packet + 4), address_of("listnode"));
        at<std::uint16_t>(owner, 378) = at<std::uint16_t>(packet, 32);
        return;
    case 5: {
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        if (copy_player(packet + 2, &player)) {
            ScopedString text;
            if (at<std::int16_t>(network_game(), 8) > 2 && player.slot >= 0) {
                at<Dword>(&player.car, 0) = 0;
                at<Dword>(&player.car, 4) =
                    at<std::uint16_t>(packet, 2) != at<std::uint16_t>(owner, 376);
                if (Dword(int(player.slot)) < 24)
                    std::memcpy(bytes(system(72)) + 248 + player.slot * 84, &player.car, 84);
                const auto phase = at<std::int16_t>(owner, 380);
                if (phase == 8 || phase == 9) {
                    mark_player_left(at<Dword *>(system(92), 4), player.slot, 1);
                    mark_player_left(at<Dword *>(system(92), 8), player.slot, 1);
                }
            }
            ScopedString name, pattern;
            gui::string_assign(&name.value, player.name);
            gui::string_make_wide(&name.value);
            gui::string_reference(&pattern.value, 1611);
            gui::string_format(&text.value, &pattern.value, gui::string_data(&name.value));
            queue::erase(player_queue(), address_of(packet + 2), nullptr);
            notice(&text.value);
        }
        if (at<std::uint16_t>(packet, 2) == at<std::uint16_t>(owner, 376)) {
            at<std::int16_t>(owner, 378) = -1;
            at<std::int16_t>(owner, 380) = 10;
        }
        return;
    }
    case 7:
        if (!route[1])
            players::apply_state(player_queue(), packet);
        if (at<std::uint16_t>(packet, 2) == at<std::uint16_t>(owner, 376)) {
            std::memcpy(bytes(owner) + 436, packet + 8, 84);
            at<std::int16_t>(owner, 380) = at<std::int16_t>(packet, 4);
            at<Dword>(system(72), 2540) = !at<Dword>(owner, 504);
            std::strcpy(reinterpret_cast<char *>(owner) + 2105,
                        vehicle_resource(system(52) + 1, at<int>(owner, 496)));
            const auto slot = at<std::int16_t>(packet, 6);
            at<std::int16_t>(owner, 378) = slot;
            at<Dword>(owner, 60) = 0;
            at<int>(system(72), 204) = slot;
        }
        return;
    case 8:
        if (at<std::uint16_t>(owner, 72)) {
            packet[0] = 27;
            send(bytes(owner) + 72, packet, 6, 4);
        } else {
            at<std::uint16_t>(owner, 72) = at<std::uint16_t>(packet, 2);
            if (!route[1])
                packets::pending::reconnect(transport, at<Dword>(owner, 24),
                                                     at<std::uint16_t>(packet, 4),
                                                     packets::pending::reconnect_port);
            packets::pending::send_join(owner);
        }
        return;
    case 11:
        if (at<std::int16_t>(owner, 378) == -1 || at<std::int16_t>(owner, 380) != 9) {
            at<std::int16_t>(owner, 380) = 8;
            at<std::int16_t>(owner, 378) = -1;
        }
        at<Dword>(owner, 60) = 0;
        reference_notice(1706);
        return;
    case 12:
        for (unsigned index = 0; index < 393; ++index)
            at<Dword>(owner, 520 + index * 4) = at<Dword>(packet, 4 + index * 4);
        at<Dword>(owner, 2092) = 1;
        return;
    case 13:
        if (size < 141 && at<Dword>(network_game(), 92)) {
            ScopedString text;
            gui::string_assign(&text.value, reinterpret_cast<char *>(packet + 8));
            notice(&text.value, at<std::int16_t>(packet, 2), at<int>(packet, 4));
        }
        return;
    case 15:
    case 34:
    case 44:
        player_update(packet);
        return;
    case 16:
        reference_notice(1344);
        at<Dword>(owner, 56) = at<Dword>(packet, 4);
        at<std::int16_t>(owner, 380) = 5;
        disconnect_route(bytes(owner) + 72);
        return;
    case 17:
        packets::pending::answer_authentication_challenge(packet + 1);
        return;
    case 29:
        rename_player(owner, packet);
        return;
    case 31:
        if (at<std::int16_t>(owner, 380) == 9 || at<std::int16_t>(owner, 380) == 8) {
            auto *object = race();
            using Method = void(__thiscall *)(Dword *, Dword);
            reinterpret_cast<const Method *>(object[0])[108 / 4](object, at<Dword>(packet, 4));
        }
        return;
    case 32:
        if (at<std::int16_t>(owner, 380) == 9 || at<std::int16_t>(owner, 380) == 8) {
            kernel::signal_wait(reinterpret_cast<kernel::Signal *>(bytes(race()) + 156));
            Dword *race_state = race();
            using Method = void(__thiscall *)(Dword *, Dword, Dword, Dword);
            const int slot = at<int>(packet, 4);
            reinterpret_cast<const Method *>(race_state[0])[120 / 4](
                race_state, slot, at<Dword>(packet, 20), at<Dword>(packet, 8));
            kernel::signal_set(reinterpret_cast<kernel::Signal *>(bytes(race()) + 156));
            if (auto *car = vehicle(slot))
                packet_native::apply_car_speeds(car, packet + 12);
        }
        return;
    case 35:
        packets::pending::receive_car_state(network_game(), packet);
        return;
    case 36:
        packets::pending::receive_position(network_game(), packet);
        return;
    case 37:
        if (!(bits::client_decoders & 1)) {
            bits::client_decoders |= 1;
            bits::construct(&bits::client_car_stream);
            std::atexit(bits::finalize_client_car);
        }
        bits::reset(&bits::client_car_stream);
        bits::bind(&bits::client_car_stream, packet, size);
        bits::decode_server_car(&bits::client_car_stream, bits::client_car);
        packets::pending::receive_car_state(network_game(), bits::client_car);
        return;
    case 38:
        if (!(bits::client_decoders & 2)) {
            bits::client_decoders |= 2;
            bits::construct(&bits::client_position_stream);
            std::atexit(bits::finalize_client_position);
        }
        bits::reset(&bits::client_position_stream);
        bits::bind(&bits::client_position_stream, packet, size);
        bits::decode_server_position(&bits::client_position_stream, bits::client_position);
        packets::pending::receive_position(network_game(), bits::client_position);
        return;
    case 39:
        ping(owner);
        return;
    case 40: {
        const Dword now = timeGetTime(), sent = at<Dword>(packet, 4);
        int elapsed = static_cast<int>(now - sent);
        if (elapsed < 0 || now < sent)
            elapsed = 0;
        else if (elapsed > 9999)
            elapsed = 9999;
        at<int>(owner, 384 + at<Byte>(owner, 424) * 4) = elapsed;
        if (++at<volatile Byte>(owner, 424) > 9)
            at<volatile Byte>(owner, 424) = 0;
        if (++at<volatile Byte>(owner, 425) > 10)
            at<volatile Byte>(owner, 425) = 10;
        const unsigned count = at<volatile Byte>(owner, 425);
        at<volatile Dword>(owner, 428) = 0;
        for (unsigned index = 0; index < count; ++index)
            at<volatile Dword>(owner, 428) += at<Dword>(owner, 384 + index * 4);
        if (at<volatile Byte>(owner, 425) > 1)
            at<volatile Dword>(owner, 428) /= count;
        if (at<Dword>(owner, 428) > 9999)
            at<Dword>(owner, 428) = 9999;
        alignas(4) Byte reply[8];
        reply[0] = 44;
        at<std::uint16_t>(reply, 2) = at<std::uint16_t>(owner, 376);
        at<int>(reply, 4) = elapsed;
        ui::PlayerRecord player;
        ui::initialize_car_settings(&player.car);
        if (copy_player(bytes(owner) + 376, &player)) {
            at<int>(&player, 132) = elapsed;
            replace_player(bytes(owner) + 376, &player);
        }
        send(bytes(owner) + 72, reply, 8, 4);
        return;
    }
    case 45: {
        int delay = static_cast<int>(Dword(packet[8]) + at<Dword>(packet, 4) - game::race_ticks);
        if (delay > 60)
            delay = 60;
        else if (delay < 1)
            return;
        if (at<std::int16_t>(network_game(), 10) > 3 && packet[1] < 24) {
            auto *motion = at<Dword *>(vehicle(packet[1]), 1768);
            if (motion && at<int>(motion, 732) < delay)
                at<int>(motion, 732) = delay;
        }
        return;
    }
    case 47:
        at<Dword>(packet, 12) = 1;
        at<Dword>(packet, 8) = 0;
        if (at<Dword>(packet, 4) < 24)
            std::memcpy(bytes(system(72)) + 248 + at<Dword>(packet, 4) * 84, packet + 8, 84);
        return;
    case 48:
        formatted_notice(packet);
        return;
    case 49:
        if (size < 269) {
            ScopedString text;
            gui::string_assign_wide(&text.value, reinterpret_cast<wchar_t *>(packet + 8));
            notice(&text.value, at<std::int16_t>(packet, 2), at<int>(packet, 4));
        }
        return;
    case 52: {
        int slot = at<int>(packet, 4);
        if (slot >= 0 && slot < 24 && system(92) && race())
            packets::pending::read_race_player(bytes(race()) + 584 + Dword(slot) * 988,
                                                        packet + 8);
        slot = at<int>(packet, 4);
        if (void *player = packets::pending::find_race_vehicle(slot))
            at<Dword>(player, 568) = at<Dword>(packet, 24);
        if (auto *car = vehicle(slot))
            packet_native::apply_car_speeds(car, packet + 28);
        return;
    }
    }
}
} // namespace nfs::network
