#include "network_lifecycle_runtime.hpp"
#include "../../frontend/screens/frontend_dialogs.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
namespace nfs::network::session {

void copy_client_route(void *owner, const void *route) {
    for (unsigned index = 0; index < 76; ++index)
        at<Dword>(owner, 72 + index * 4) = at<Dword>(const_cast<void *>(route), index * 4);
    at<Dword>(owner, 80) = 1;
}

Dword begin_client_connection(void *, Dword address, std::uint16_t port) {
    alignas(4) Byte route[304];
    if (at<Dword>(server(), 44)) {
        std::memset(route, 0, sizeof(route));
        at<Dword>(route, 12) = 1;
        at<Dword>(route, 4) = 1;
        const Dword identifier = at<Dword>(server(), 220)++;
        at<std::uint16_t>(route, 0) = std::uint16_t(identifier);
        copy_client_route(client(), route);
        queue::append(route_queue(), reinterpret_cast<Dword>(route),
                      reinterpret_cast<Dword>("listnode"));
        packets::pending::send_join(client());
        return 1;
    }
    if (address) {
        std::memset(route, 0, sizeof(route));
        copy_client_route(client(), route);
        if (connect_transport(transport, address, port, services::dependency::local_tcp_port))
            return 1;
    }
    return 0;
}

Dword pump_connection_dialog(void *widget) {
    if (!gui::context_state)
        return 11;
    gui::pump_context(gui::context_state);
    const Dword status = gui::poll_context(widget);
    if (status)
        return status;
    for (void *parent = at<void *>(widget, 56); parent; parent = at<void *>(parent, 56))
        widget = parent;
    return gui::tick_root(widget);
}

void __cdecl wait_for_connection(void *widget) {
    pump_connection_dialog(widget);
    void *owner = client();
    if (!begin_client_connection(owner, at<Dword>(owner, 24), at<std::uint16_t>(owner, 28))) {
        at<std::uint16_t>(client(), 380) = 3;
        gui::events::dispatch(widget, 24, 2, at<Dword>(widget, 4));
        return;
    }
    const Dword start = transport_time();
    std::int16_t state;
    do {
        pump_connection_dialog(widget);
        kernel::thread_yield(1);
        const Dword now = transport_time();
        if (static_cast<std::int32_t>(now - (start + 15000)) > 0)
            at<std::uint16_t>(client(), 380) = 3;
        state = at<std::int16_t>(client(), 380);
    } while (state == 2 || state == 4 || state == 6);
    gui::events::dispatch(widget, 24, 2, at<Dword>(widget, 4));
}

void client_route_closed(void *owner) {
    if (!at<Dword>(owner, 84))
        return;
    std::memset(bytes(owner) + 72, 0, 304);
    if (at<std::int16_t>(owner, 380) == 10) {
        at<std::uint16_t>(owner, 380) = 1;
        at<Dword>(owner, 56) = 16;
        return;
    }
    if (!at<Dword>(owner, 56)) {
        at<std::uint16_t>(owner, 380) = 3;
        at<Dword>(owner, 56) = 15;
        packets::ScopedString message;
        gui::string_reference(&message.value, 1700);
        packets::notice(&message.value);
        at<std::uint16_t>(owner, 2096) = 0;
        at<std::uint16_t>(owner, 28) = 0;
        at<Dword>(owner, 24) = 0;
    }
}
} // namespace nfs::network::session
namespace nfs::services::pending {

void connect_client(void *owner, const char *address, Dword port, Dword, Dword quiet_password) {
    using namespace network::session;
    if (at<Dword>(owner, 84))
        network::disconnect_client(static_cast<Dword *>(owner), 16);
    at<const char *>(owner, 24) = address;
    at<std::uint16_t>(owner, 28) = std::uint16_t(port);
    at<std::uint16_t>(owner, 376) = 0;
    std::memset(bytes(owner) + 384, 0, 40);
    at<volatile Byte>(owner, 424) = 0;
    at<volatile Byte>(owner, 425) = 0;
    void *frontend = system(96), *root = nullptr;
    if (frontend && (root = at<void *>(frontend, 40)))
        application::pending::widget_root(static_cast<application::pending::Widget *>(root));
    at<std::uint16_t>(owner, 380) = 2;
    at<Dword>(owner, 56) = 0;
    application::pending::MessageBox box;
    {
        network::packets::ScopedString title, text;
        gui::string_reference(&title.value, 1702);
        gui::string_reference(&text.value, 1701);
        application::pending::construct_message_box(&box, &text.value, &title.value, 0);
    }
    try {
        application::pending::run_message_box(&box, wait_for_connection, 0);
        if (at<std::int16_t>(owner, 380) == 7)
            at<Dword>(owner, 68) = 1;
        else {
            if (!quiet_password || at<Dword>(owner, 56) != 2)
                show_connection_error(owner, root, 1);
            at<Dword>(owner, 68) = 0;
            reset_network_session(network::network_game());
        }
    } catch (...) {
        application::pending::destroy_message_box(&box);
        throw;
    }
    application::pending::destroy_message_box(&box);
}

void disconnect_failed_client(void *owner) {
    using namespace network::session;
    if (at<std::int16_t>(network::network_game(), 10) > 3 && !at<Dword>(network::server(), 44) &&
        game::tick_services[0] && !at<Dword>(game::tick_services[0], 300)) {
        if (at<Dword>(network::network_game(), 16) &&
            network::players::selected_players(player_queue())) {
            transition_network(network::network_game());
            game::world::pending::reset_network_camera(network::network_game());
            if (at<Dword>(game::tick_services[0], 300))
                return;
        }
        network::disconnect_client(static_cast<Dword *>(owner), 16);
    }
}
} // namespace nfs::services::pending
