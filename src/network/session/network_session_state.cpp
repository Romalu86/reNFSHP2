#include "nfs/network/session/network_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include <mmsystem.h>
namespace nfs::network {
Dword *transport = nullptr;
Dword *server() {
    return reinterpret_cast<Dword *>(lifecycle::manager->slots[108 / 4]);
}
Dword *client() {
    return reinterpret_cast<Dword *>(lifecycle::manager->slots[112 / 4]);
}
Dword *server_queue(Dword *owner) {
    return reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(owner) +
                                    reinterpret_cast<Dword *>(owner[1])[1] + 4);
}

Dword *network_game() {
    return reinterpret_cast<Dword *>(lifecycle::manager->slots[104 / 4]);
}

Dword session_active() {
    return (server()[44 / 4] || client()[68 / 4]) ? 1 : 0;
}

Dword render_mode() {
    return graphics::mode;
}
// TimeGetTime tail entry has no receiver reads.
Dword transport_time() {
    return timeGetTime();
}
} // namespace nfs::network
