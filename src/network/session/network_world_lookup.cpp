#include "nfs/network/session/network_world.hpp"
namespace nfs::network::world {
void *vehicle_interface(Dword value) {
    auto *object = reinterpret_cast<Dword *>(value);
    auto *receiver = reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(object) +
                                              reinterpret_cast<Dword *>(object[1])[2] + 4);
    using Get = void *(__thiscall *)(Dword *);
    return reinterpret_cast<const Get *>(receiver[0])[2](receiver);
}
} // namespace nfs::network::world
namespace nfs::network::packets::pending {

void *find_race_vehicle(int slot) {
    world::Iterator iterator;
    world::construct_iterator(&iterator, world::vehicle_registry);
    for (Dword item = world::first(&iterator); item; item = world::next(&iterator))
        if (void *vehicle = world::vehicle_interface(item))
            if (world::vehicle_slot(vehicle) == Dword(slot))
                return vehicle;
    return nullptr;
}
} // namespace nfs::network::packets::pending
