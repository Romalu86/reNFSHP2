#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/network/packets/network_quantizers.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/network/packets/network_authentication.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../packets/network_packet_x87.hpp"
#include <cstring>
#include <cstdlib>
#include <mmsystem.h>

namespace nfs::services {
namespace {
void *system(unsigned byte_offset) {
    return lifecycle::manager->slots[byte_offset / 4];
}
} // namespace

void *construct_transport(void *object) {
    field<const Dword *>(object, 0) = pending::transport_methods;
    for (unsigned offset :
         {40u, 44u, 48u, 56u, 52u, 4u, 16u, 8u, 20u, 28u, 24u, 32u, 12u})
        field<Dword>(object, offset) = 0;
    field<std::uint16_t>(object, 36) = 8511;
    field<std::uint16_t>(object, 38) = 8512;
    return object;
}

void destroy_transport(void *object) {
    field<const Dword *>(object, 0) = pending::transport_methods;
}

void bind_transport_messages(void *object, void *messages) {
    field<void *>(object, 56) = messages;
}

void *construct_network_messages(void *object) {
    field<const Dword *>(object, 0) = pending::message_methods;
    return object;
}

void destroy_network_messages(void *object) {
    field<const Dword *>(object, 0) = pending::message_methods;
}

void initialize_network(void *object) {
    if (!network::transport) {
        void *storage = lifecycle::allocate(84, "Net", lifecycle::level_pool);
        network::transport = static_cast<Dword *>(storage ? construct_transport(storage) : nullptr);
        pending::prepare_network_transport(network::transport);
    }
    if (!pending::network_messages) {
        void *storage = lifecycle::allocate(4, "NetMessage", lifecycle::level_pool);
        pending::network_messages = storage ? construct_network_messages(storage) : nullptr;
    }
    bind_transport_messages(network::transport, pending::network_messages);
    field<std::uint16_t>(object, 8) = 0;
    field<std::uint16_t>(object, 10) = 0;
    pending::initialize_network_identity();
    pending::load_network_settings();
    network::bits::configure_quantizers();
    pending::initialize_network_filters();
    pending::reset_network_filters();
}

void shutdown_network(void *) {
    if (network::transport)
        pending::release_network_settings();
    if (void *message = pending::network_messages) {
        destroy_network_messages(message);
        gui::application_release(message);
    }
    pending::network_messages = nullptr;
    if (network::transport)
        pending::stop_network_transport(network::transport);
    if (void *transport = network::transport) {
        destroy_transport(transport);
        gui::application_release(transport);
    }
    network::transport = nullptr;
    pending::shutdown_network_identity();
}

void finish_network_round(void *object) {
    if (field<Dword>(system(108), 44) || field<Dword>(system(112), 68)) {
        void *rules = system(72);
        field<Dword>(rules, 204) = field<Dword>(rules, 208) = 4294967295u;
        if (field<Dword>(system(112), 68) && field<std::int16_t>(object, 10) > 3) {
            const Dword start = timeGetTime();
            Dword now;
            do {
                do {
                    kernel::cooperate(0);
                    kernel::thread_yield(1);
                    now = timeGetTime();
                } while (now < start + 500);
                now = timeGetTime();
            } while (now < start + 3000 && field<int>(system(72), 168) < 1);
            if (field<int>(system(72), 168) < 1)
                pending::disconnect_failed_client(system(112));
            field<Dword>(system(72), 172) = 0;
        } else {
            field<Dword>(rules, 168) = field<Dword>(rules, 172) = 0;
        }
        network::packets::pending::set_game_rule(static_cast<Dword *>(system(72)), 14, 0);
        field<Dword>(system(72), 184) = 0;
    }
}


void initialize_server(void *object) {
    network::reset_server(object);
    if (!pending::server_mode)
        return;
    network::packets::pending::session_kind = static_cast<int>(pending::network_kind);
    pending::load_server_settings(pending::server_settings);
    auto setting = [](unsigned offset) -> int & {
        return field<int>(pending::server_settings, offset);
    };
    if (setting(524) == 5 || setting(524) == 11 || setting(524) == 8 || setting(524) == 2) {
        set_server_race_setting(pending::server_settings, 0);
        setting(672) = 1;
    } else if (setting(680) == 1) {
        setting(672) = static_cast<int>(Dword(setting(668)) - 1);
        if (setting(672) < 1)
            setting(672) = 1;
    }
    if (lifecycle::dedicated)
        field<Byte>(pending::server_settings, 692) = 1;
    field<Dword>(object, 56) = setting(680) == 1 ? 1 : 0;
    field<Dword>(object, 60) = setting(688);
    field<Dword>(object, 64) = setting(524);
    field<Dword>(object, 72) = setting(672);
    field<Dword>(object, 68) = setting(684);
    field<Byte>(object, 96) = static_cast<Byte>(setting(668));
    std::strncpy(static_cast<char *>(object) + 172,
                 reinterpret_cast<char *>(pending::server_settings), 24);
    field<Byte>(object, 195) = 0;
    std::strncpy(static_cast<char *>(object) + 196,
                 reinterpret_cast<char *>(pending::server_settings) + 632, 24);
    field<Byte>(object, 219) = 0;
    network::packets::set_server_name(static_cast<Dword *>(system(104)),
                                      static_cast<char *>(object) + 172);
    pending::activate_server(
        object, pending::dedicated_server || field<Byte>(pending::server_settings, 692) ? 1 : 0);
}


void initialize_client(void *object) {
    field<std::uint16_t>(object, 380) = 1;
    field<Dword>(object, 56) = 0;
    field<std::uint16_t>(object, 2096) = 0;
    field<Byte>(object, 2104) = 0;
    field<Dword>(object, 24) = 0;
    field<std::uint16_t>(object, 28) = field<std::uint16_t>(object, 376) = 0;
    field<std::uint16_t>(object, 378) = 65535;
    std::memset(static_cast<Byte *>(object) + 2105, 0, 32);
    std::memset(static_cast<Byte *>(object) + 436, 0, 84);
    std::memset(static_cast<Byte *>(object) + 30, 0, 24);
    if (pending::player_name_requested && *pending::player_name)
        std::strncpy(static_cast<char *>(object) + 30, pending::player_name, 24);
    if (pending::join_mode &&
        (!field<Dword>(system(108), 44) || !field<Dword>(system(108), 48))) {
        field<Dword>(object, 68) = 1;
        pending::configure_client(object, 0);
        gui::String name;
        gui::string_construct(&name);
        try {
            gui::string_copy(&name,
                             reinterpret_cast<gui::String *>(static_cast<Byte *>(object) + 4));
            gui::string_make_narrow(&name);
            std::strncpy(static_cast<char *>(object) + 460, gui::string_ascii(&name), 26);
            field<Byte>(object, 485) = 0;
            const Dword port = pending::port_requested ? std::atoi(pending::port_text) : 0;
            pending::connect_client(object, pending::join_address, port, 0, 0);
        } catch (...) {
            gui::string_destruct(&name);
            throw;
        }
        gui::string_destruct(&name);
    }
    field<Dword>(object, 436) = field<Dword>(object, 504) = 1;
    field<Dword>(object, 444) = field<Dword>(object, 452) = 0;
    if (pending::car_override) {
        field<Dword>(object, 496) =
            find_catalog_car(static_cast<Byte *>(system(52)) + 4, pending::car_name);
        field<Dword>(object, 60) = 1;
    }
}

void update_network(void *object) {
    if (!network::session_active())
        return;
    pending::transition_network(object);
    if (field<std::int16_t>(object, 10) > 3) {
        int count = field<int>(system(72), 168);
        frame_clock::step = 1.0f;
        field<Dword>(object, 80) = 0;
        for (unsigned offset = 20; count > 0; --count, offset += 4) {
            void *car = field<void *>(system(68), offset);
            if (car && pending::car_is_remote(car)) {
                void *client_game = field<void *>(system(92), 4);
                if (field<Dword>(object, 48) ||
                    pending::client_player_left(client_game, field<Dword>(car, 1772)))
                    field<Dword>(field<void *>(car, 1768), 732) = 90;
                pending::update_remote_car(static_cast<Byte *>(object) + 76, game::race_ticks,
                                           car);
            }
        }
        const Dword latest = field<Dword>(object, 80);
        if (latest) {
            const Dword negative_elapsed_ticks =
                network::packet_native::clock_ticks(frame_clock::elapsed, -60.0f);
            const Dword adjustment =
                3u - negative_elapsed_ticks - graphics::unresolved::camera_state;
            if (field<int>(system(72), 204) == -1) {
                if (latest < game::race_ticks + 20u + adjustment)
                    frame_clock::step = 0.25f;
                else if (game::race_ticks + 105u + adjustment < latest)
                    frame_clock::step = 1.5f;
            } else if (game::race_ticks + adjustment < latest)
                frame_clock::step = 2.0f;
        }
    }
    if (field<Dword>(system(108), 44))
        pending::tick_server(system(108));
    if (field<Dword>(system(112), 68))
        pending::tick_client(system(112));
    if (network::packets::pending::session_kind == 1) {
        pending::tick_server_browser();
        pending::tick_internet_services();
    }
}
} // namespace nfs::services
