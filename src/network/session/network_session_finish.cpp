#include "network_lifecycle_runtime.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::network::session {
void reset_transport_errors(void *);
}
namespace nfs::application::pending {

void finish_network_race_session(void *owner) {
    using namespace network::session;
    using network::client;
    using network::server;
    using network::server_queue;
    using network::transport;
    if (at<Dword>(server(), 44))
        network::leave_game(server());
    else {
        if (!at<Dword>(client(), 68))
            return;
        if (at<Dword>(server(), 44))
            network::leave_game(server());
    }
    if (at<Dword>(client(), 68))
        at<Dword>(client(), 68) = 0;
    at<std::uint16_t>(owner, 8) = 0;
    at<std::uint16_t>(owner, 10) = 0;
    at<Dword>(owner, 48) = 0;
    at<Byte>(owner, 44) = 0;
    for (unsigned offset : {12u, 20u, 24u, 28u, 16u, 32u, 36u, 92u})
        at<Dword>(owner, offset) = 0;
    std::memset(bytes(owner) + 52, 0, 24);
    lifecycle::construction::network::queue::clear(server_queue(static_cast<Dword *>(owner)));
    reset_transport_errors(transport);
    at<Dword>(owner, 80) = 0;
    game::runtime::reset_complete = 0;
    game::phase_reset = 0;
    game::runtime::reset_requested = 0;
    game::phase_pending = 0;
    game::phase_locked = 0;
    network::packets::pending::reset_lobby();
}
} // namespace nfs::application::pending
