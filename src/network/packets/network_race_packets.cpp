#include "nfs/network/vehicles/network_snapshots.hpp"
namespace nfs::network::snapshots {

void *clear_vector3(void *vector) {
    field<Dword>(vector, 0) = 0;
    field<Dword>(vector, 4) = 0;
    field<Dword>(vector, 8) = 0;
    return vector;
}
} // namespace nfs::network::snapshots
namespace nfs::network::packets::pending {
// Unwritten record and packet fields are preserved.
void read_race_player(void *player, void *packet) {
    at<Dword>(player, 48) = at<Dword>(packet, 24);
    at<Dword>(player, 44) = at<Dword>(packet, 20);
    at<Byte>(player, 940) = at<Byte>(packet, 0);
    at<Byte>(player, 941) = at<Byte>(packet, 1);
    at<Dword>(player, 880) = at<Dword>(packet, 108);
    for (unsigned index = 0; index < 10; ++index) {
        at<Dword>(player, 456 + index * 4) = at<Dword>(packet, 68 + index * 4);
        at<Dword>(player, 56 + index * 4) = at<Dword>(packet, 28 + index * 4);
    }
    at<Byte>(player, 942) = at<Byte>(packet, 2);
    at<Dword>(player, 888) = at<Dword>(packet, 112);
    at<Dword>(player, 8) = at<Dword>(packet, 4);
    const Dword third = at<Dword>(packet, 12);
    at<Dword>(player, 12) = at<Dword>(packet, 8);
    at<Dword>(player, 16) = third;
}

void write_race_player(void *player, void *packet) {
    at<Dword>(packet, 24) = at<Dword>(player, 48);
    at<Dword>(packet, 20) = at<Dword>(player, 44);
    at<Byte>(packet, 0) = at<Byte>(player, 940);
    at<Byte>(packet, 1) = at<Byte>(player, 941);
    for (unsigned index = 0; index < 10; ++index) {
        at<Dword>(packet, 68 + index * 4) = at<Dword>(player, 456 + index * 4);
        at<Dword>(packet, 28 + index * 4) = at<Dword>(player, 56 + index * 4);
    }
    at<Dword>(packet, 108) = at<Dword>(player, 880);
    at<Byte>(packet, 2) = at<Byte>(player, 942);
    at<Dword>(packet, 112) = at<Dword>(player, 888);
    at<Dword>(packet, 4) = at<Dword>(player, 8);
    const Dword third = at<Dword>(player, 16);
    at<Dword>(packet, 8) = at<Dword>(player, 12);
    at<Dword>(packet, 12) = third;
}
} // namespace nfs::network::packets::pending
