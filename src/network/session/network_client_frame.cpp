#include "network_lifecycle_runtime.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../race/session/race_runtime.hpp"
#include "../../frontend/screens/frontend_dialogs.hpp"
#include "../packets/network_packet_x87.hpp"
namespace nfs::services::pending {

void tick_client(void *owner) {
    using namespace network::session;
    bool phase_changed = false;
    alignas(4) Byte message[8];
    auto send_small = [&](Byte opcode, std::uint16_t identifier, Dword value, Dword size,
                          Dword flags) {
        message[0] = opcode;
        at<std::uint16_t>(message, 2) = identifier;
        // Only the payload bytes written by the case are assigned below.
        if (size == 8)
            at<Dword>(message, 4) = value;
        else if (opcode == 34)
            at<Byte>(message, 4) = Byte(value);
        else
            at<std::uint16_t>(message, 4) = std::uint16_t(value);
        network::packets::send(bytes(owner) + 72, message, size, flags);
    };
    if (!at<Dword>(owner, 84)) {
        application::pending::widget_root(
            at<application::pending::Widget *>(network::network_game(), 92));
        at<std::uint16_t>(network::network_game(), 8) = 1;
        if (!at<std::uint16_t>(owner, 376) || at<Dword>(network::server(), 44))
            return;
        void *root = at<void *>(system(96), 40);
        if (!root || !method<Dword>(root, 3, "SimGUIInterface"))
            return;
        root = at<void *>(system(96), 40);
        at<std::uint16_t>(owner, 376) = 0;
        gui::events::propagate(root, 110, 0, 0, 2);
        at<std::uint16_t>(owner, 2096) = 0;
        return;
    }
    if (!at<Dword>(owner, 76)) {
        Dword now = network::transport_time();
        if (at<std::int16_t>(owner, 380) == 7) {
            at<Dword>(owner, 92) = now;
            send_small(27, at<std::uint16_t>(owner, 72),
                       at<std::uint16_t>(network::transport, 38), 6, 4);
        }
        if (static_cast<std::int32_t>(now - at<Dword>(owner, 432)) > 10000) {
            void *current = network::client();
            const Dword timestamp = network::transport_time();
            at<Dword>(current, 432) = network::transport_time();
            message[0] = 39;
            at<Dword>(message, 4) = timestamp;
            network::packets::send(bytes(current) + 72, message, 8, 0);
        }
        if (static_cast<std::int32_t>(now - at<Dword>(network::transport, 28)) > 5000) {
            void *current = network::client();
            const Dword timestamp = network::transport_time();
            at<Dword>(current, 432) = network::transport_time();
            message[0] = 39;
            at<Dword>(message, 4) = timestamp;
            network::packets::send(bytes(current) + 72, message, 8, 4);
        }
        now = network::transport_time();
        if (static_cast<std::int32_t>(now - at<Dword>(owner, 92)) > 30000)
            network::disconnect_client(static_cast<Dword *>(owner), 7);
    }
    if (at<Byte>(owner, 2104) != Byte(at<Dword>(system(36), 48))) {
        const Byte frame = Byte(at<Dword>(system(36), 48));
        at<Byte>(owner, 2104) = frame;
        send_small(34, at<std::uint16_t>(owner, 376), frame, 6, 0);
    }
    if (at<std::uint16_t>(owner, 2096) != at<std::uint16_t>(network::network_game(), 8) ||
        at<std::int16_t>(owner, 380) == 7) {
        at<std::uint16_t>(owner, 2096) = at<std::uint16_t>(network::network_game(), 8);
        phase_changed = true;
        send_small(28, at<std::uint16_t>(owner, 376), at<std::uint16_t>(owner, 2096), 6, 0);
    }
    disconnect_failed_client(owner);
    switch (at<std::uint16_t>(owner, 380)) {
    case 3: {
        at<std::uint16_t>(owner, 380) = 1;
        at<Dword>(owner, 56) = 15;
        network::packets::ScopedString text;
        gui::string_reference(&text.value, 1700);
        network::packets::notice(&text.value);
        break;
    }
    case 7:
        at<std::uint16_t>(owner, 380) = 8;
        at<Dword>(owner, 432) = 0;
        [[fallthrough]];
    case 8:
        at<Dword>(owner, 64) = 0;
        if (phase_changed && at<std::int16_t>(network::network_game(), 8) == 2) {
            const Dword timestamp = network::transport_time();
            at<Dword>(owner, 432) = network::transport_time();
            message[0] = 39;
            at<Dword>(message, 4) = timestamp;
            network::packets::send(bytes(owner) + 72, message, 8, 4);
            gui::events::propagate(at<void *>(system(96), 40), 111, 0, 0, 2);
        }
        select_spectator_vehicle(owner);
        return;
    case 9: {
        const float elapsed = frame_clock::elapsed, previous = at<float>(owner, 64);
        const bool changed = network::packet_native::snapshot_due(elapsed, previous) != 0;
        if (changed)
            at<float>(owner, 64) = elapsed;
        if ((game::start_deadline || game::runtime::race_started) && changed &&
            at<std::int16_t>(owner, 378) >= 0)
            send_client_snapshot(owner);
        return;
    }
    }
}
} // namespace nfs::services::pending
