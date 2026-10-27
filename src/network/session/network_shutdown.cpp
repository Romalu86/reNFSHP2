#include "nfs/network/session/network_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/frontend/network/frontend_network.hpp"
namespace nfs::network {
Dword saved_graphics_enabled = 0;
 
 
void leave_game(Dword *owner) {
    namespace q = lifecycle::construction::network::queue;
    field<std::uint16_t>(owner, 12) = 1;
    set_game_phase(network_game(), field<std::int16_t>(owner, 12));
    alignas(4) std::uint8_t route[304];
    for (int index = 0; index < static_cast<int>(q::count(server_queue(owner))); ++index) {
        if (!q::copy_nth(server_queue(owner), 0, index, 0, reinterpret_cast<Dword>(route), nullptr,
                         0, nullptr))
            continue;
        while (remove_server_client(owner, nullptr, field<Dword>(route, 0))) {
        }
        alignas(4) std::uint8_t packet[8]; // Bytes 1 through 3 remain uninitialized.
        packet[0] = 16;
        field<Dword>(packet, 4) = 3;
        send_packet(reinterpret_cast<Dword>(route), reinterpret_cast<Dword>(packet), 8, 0);
        field<Dword>(route, 12) = 0;
    }
    const Dword deadline = transport_time() + 500;
    const Dword dedicated = field<Dword>(owner, 48);
    do {
        if (!dedicated && render_mode() == 3)
            kernel::cooperate(0);
        kernel::thread_yield(1);
    } while (transport_time() < deadline);
    field<Dword>(owner, 44) = 0;
    while (q::take(server_queue(owner), 0, reinterpret_cast<Dword>(route)))
        if (field<Dword>(route, 36))
            destroy_packet(field<Dword *>(route, 36));
    q::clear(field<Dword *>(owner, 312));
    if (field<Dword>(owner, 48)) {
        graphics_enabled = saved_graphics_enabled;
        if (!has_instance_mutex())
            acquire_instance_mutex();
    }
    reset_server(owner);
    frontend::clear_trackers_entry();
    frontend::exit();
}
} // namespace nfs::network
