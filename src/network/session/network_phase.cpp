#include "nfs/network/session/network_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/gui/events/gui_events.hpp"
#include "nfs/network/session/network_player_state.hpp"
#include "../packets/network_packet_x87.hpp"
namespace nfs::network {
namespace pending {} // namespace pending
// Phase 1 and unknown phases require no action.
void set_game_phase(Dword *owner, std::int16_t phase) {
    namespace q = lifecycle::construction::network::queue;
    switch (phase) {
    case 2:
        field<Dword>(owner, 24) = 1;
        field<std::uint16_t>(owner, 8) = 0;
        return;
    case 3: {
        auto *signal =
            reinterpret_cast<kernel::Signal *>(reinterpret_cast<std::uint8_t *>(owner) + 84);
        kernel::signal_wait(signal);
        const auto previous = field<std::int16_t>(owner, 8), last = field<std::int16_t>(owner, 10);
        game::phase_pending = 0;
        if (previous == 2 && !game::phase_locked && client()[68 / 4])
            field<std::uint16_t>(client(), 2096) = 0;
        field<std::uint16_t>(owner, 8) = 2;
        if (last < 4) {
            if (previous > 1)
                q::visit(server_queue(owner), 0, 0, reinterpret_cast<Dword *>(&players::ready), 0,
                         nullptr);
        } else
            field<Dword>(owner, 20) = 1;
        kernel::signal_set(signal);
        return;
    }
    case 4:
        if (field<std::int16_t>(owner, 8) != 4) {
            field<Dword>(owner, 16) = 1;
            field<std::uint16_t>(owner, 8) = 3;
        }
        return;
    case 5:
        if (server()[44 / 4])
            q::visit(server_queue(owner), 0, 0, reinterpret_cast<Dword *>(&players::start), 0,
                     nullptr);
        field<std::uint16_t>(owner, 8) = 5;
        game::phase_reset = 0;
        graphics::unresolved::camera_state =
            packet_native::clock_ticks(frame_clock::elapsed, 60.0f);
        game::start_deadline = game::countdown_ticks + graphics::unresolved::camera_state;
        return;
    case 6:
        field<std::uint16_t>(owner, 8) = 6;
        if (server()[44 / 4]) {
            void *gui =
                field<void *>(reinterpret_cast<Dword *>(lifecycle::manager->slots[96 / 4]), 40);
            nfs::gui::events::propagate(gui, 112, 0, 0, 2);
        } else
            field<Dword>(owner, 28) = 1;
        return;
    }
}
} // namespace nfs::network
