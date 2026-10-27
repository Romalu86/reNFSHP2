#include "nfs/network/packets/network_packet_handlers.hpp"
#include <cstring>
namespace nfs::network::packets {

const char *vehicle_resource(Dword *catalogue, int identifier) {
    for (int index = 0; index < static_cast<int>(catalogue[0]); ++index)
        if (static_cast<int>(catalogue[1 + index * 307]) == identifier)
            return reinterpret_cast<char *>(catalogue + 2 + index * 307);
    return nullptr;
}

int vehicle_class(Dword *catalogue, int identifier) {
    for (int index = 0; index < static_cast<int>(catalogue[0]); ++index)
        if (static_cast<int>(catalogue[1 + index * 307]) == identifier)
            return catalogue[8 + index * 307];
    return 42;
}

Dword rule_value(Dword *rules, int index) {
    return **reinterpret_cast<Dword **>(bytes(rules) + 102668 + Dword(index) * 4);
}

void set_server_name(Dword *game, const char *name) {
    if (name) {
        std::strncpy(reinterpret_cast<char *>(game) + 52, name, 24);
        at<Byte>(game, 75) = 0;
    }
}
// Only the first seven words are written.
void clear_server_settings(void *settings) {
    for (int index = 6; index >= 0; --index)
        at<Dword>(settings, index * 4) = 0;
}
// Only AL is the player-left flag.
Dword player_left(Dword *object, int slot) {
    const Dword offset = Dword(slot) * 988;
    return (offset & 4294967040u) | at<Byte>(object, offset + 1526);
}
// Exact 13-word table at .
Dword track_reference(int track) {
    static const Dword references[] = {2617, 2618, 2619, 2620, 2621, 2622, 2623,
                                      2624, 2625, 2626, 2627, 2628, 2613};
    return track >= 0 && track < 13 ? references[track] : 2613;
}
} // namespace nfs::network::packets
