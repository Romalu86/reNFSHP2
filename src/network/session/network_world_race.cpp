#include "nfs/network/session/network_world.hpp"
namespace nfs::network::world {
using namespace packets;
namespace pending {
const Dword race_player_methods[] = {reinterpret_cast<Dword>(delete_race_player)};
}
// Sole race-player table entry;  begins another class table.
void *__fastcall delete_race_player(void *player, void *, unsigned flags) {
    destroy_race_player(player);
    if (flags & 1)
        gui::application_release(player);
    return player;
}
// Includes the two 100-word tables and owned empty String.
void *copy_race_player(void *destination, const void *input) {
    auto *source = const_cast<void *>(input);
    const unsigned offsets[] = {4, 8, 12, 16, 20, 28, 32, 36, 44, 48, 52};
    for (unsigned offset : offsets)
        at<Dword>(destination, offset) = at<Dword>(source, offset);
    gui::string_construct(reinterpret_cast<gui::String *>(bytes(destination) + 860));
    for (unsigned offset = 880; offset <= 888; offset += 4)
        at<Dword>(destination, offset) = at<Dword>(source, offset);
    at<Dword>(destination, 936) = at<Dword>(source, 936);
    for (unsigned offset = 940; offset <= 942; ++offset)
        at<Byte>(destination, offset) = at<Byte>(source, offset);
    at<Dword>(destination, 944) = at<Dword>(source, 944);
    at<const Dword *>(destination, 0) = pending::race_player_methods;
    for (unsigned index = 0; index < 100; ++index) {
        at<Dword>(destination, 56 + index * 4) = at<Dword>(source, 56 + index * 4);
        at<Dword>(destination, 456 + index * 4) = at<Dword>(source, 456 + index * 4);
    }
    for (unsigned index = 0; index < 10; ++index)
        at<Dword>(destination, 892 + index * 4) = at<Dword>(source, 892 + index * 4);
    for (unsigned index = 0; index < 5; ++index) {
        at<Dword>(destination, 948 + index * 4) = at<Dword>(source, 948 + index * 4);
        at<Dword>(destination, 968 + index * 4) = at<Dword>(source, 968 + index * 4);
    }
    return destination;
}

void destroy_race_player(void *player) {
    at<const Dword *>(player, 0) = pending::race_player_methods;
    gui::string_destruct(reinterpret_cast<gui::String *>(bytes(player) + 860));
}
} // namespace nfs::network::world
namespace nfs::network::packets::pending {

void send_race_players(Dword *owner, void *target) {
    alignas(4) Byte packet[124];
    packet[0] = 52;
    world::Iterator iterator;
    world::construct_iterator(&iterator, world::vehicle_registry);
    for (Dword item = world::first(&iterator); item; item = world::next(&iterator)) {
        void *object = world::vehicle_interface(item);
        if (!object)
            continue;
        const Dword slot = world::vehicle_slot(object);
        at<Dword>(packet, 4) = slot;
        alignas(4) Byte player[988];
        world::copy_race_player(player, bytes(race()) + 584 + slot * 988);
        try {
            write_race_player(player, packet + 8);
            at<Dword>(packet, 24) = at<Dword>(object, 568);
            if (void *car = vehicle(slot)) {
                at<Dword>(packet, 32) = at<Dword>(car, 1856);
                at<Dword>(packet, 28) = at<Dword>(car, 1852);
            }
            if (target)
                send(target, packet, sizeof(packet));
            else
                broadcast(owner, packet, sizeof(packet), 0, nullptr);
        } catch (...) {
            world::destroy_race_player(player);
            throw;
        }
        world::destroy_race_player(player);
    }
}
} // namespace nfs::network::packets::pending
