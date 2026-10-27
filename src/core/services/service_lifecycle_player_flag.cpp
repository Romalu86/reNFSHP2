#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
namespace nfs::services::pending {
Byte client_player_left(void *object, Dword slot) {
    return Byte(network::packets::player_left(static_cast<Dword *>(object), static_cast<int>(slot)));
}
} // namespace nfs::services::pending
