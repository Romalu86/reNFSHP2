#pragma once
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::network::world {
struct Iterator {
    const Dword *methods;
    Dword *array;
    Dword index;
};
extern Dword *&vehicle_registry;
Iterator *construct_iterator(Iterator *, Dword *);
Dword first(Iterator *);
Dword next(Iterator *);
std::uint8_t iterator_done(Iterator *);
Dword vehicle_slot(void *);
void *vehicle_interface(Dword);
void *copy_race_player(void *, const void *);
void destroy_race_player(void *);
void *__fastcall delete_race_player(void *, void *, unsigned);
namespace pending {
extern const Dword race_player_methods[];
}
} // namespace nfs::network::world
