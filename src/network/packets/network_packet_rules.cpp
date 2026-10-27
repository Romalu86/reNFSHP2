#include "nfs/network/packets/network_packet_handlers.hpp"
#include <cstring>
namespace nfs::network::packets::pending {

void set_game_rule(Dword *rules, int index, Dword value) {
    **reinterpret_cast<Dword **>(bytes(rules) + 102668 + Dword(index) * 4) = value;
}

void select_track(Dword *tracks, Dword index) {
    at<Dword>(tracks, 8) = index;
    std::strncpy(reinterpret_cast<char *>(tracks) + 140, at<const char *>(tracks, 12 + index * 4),
                 12);
}
} // namespace nfs::network::packets::pending
