#include "network_lifecycle_runtime.hpp"
#include "../../frontend/screens/frontend_dialogs.hpp"
#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/core/application/application.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/frontend/network/frontend_network.hpp"
#include <cstdio>
namespace nfs::network::session {
namespace {
Dword &query_status = frontend::query_identifier;
Dword heartbeat_clock{}, heartbeat_elapsed{};
Byte heartbeat_guard{};
Byte &awaiting_heartbeat = frontend::state[0];
} // namespace

void initialize_server_query() {
    const bool internet = packets::pending::session_kind == 1;
    std::strcpy(pending::master_host, internet ? "master.gamespy.com" : "1.2.3.4");
    const auto port =
        internet ? services::dependency::internet_query_port : services::dependency::lan_query_port;
    query_status = pending::initialize_query(
        0, 0, port, "nfs6", service_key, reinterpret_cast<void *>(pending::server_query_parameters),
        reinterpret_cast<void *>(pending::server_query_players),
        reinterpret_cast<void *>(pending::server_query_rules),
        reinterpret_cast<void *>(pending::server_query_echo), 0);
    if (!query_status) {
        heartbeat_elapsed = 0;
        if (packets::pending::session_kind == 1)
            awaiting_heartbeat = 1;
    }
}

void update_server_query() {
    if (query_status)
        return;
    const Dword now = GetTickCount();
    if (!(heartbeat_guard & 1)) {
        heartbeat_guard |= 1;
        heartbeat_clock = GetTickCount();
    }
    const Dword delta = now - heartbeat_clock, sign = Dword(static_cast<std::int32_t>(delta) >> 31);
    const Dword elapsed = (delta ^ sign) - sign;
    if (elapsed > 100) {
        if (elapsed < 500)
            heartbeat_elapsed += elapsed;
        heartbeat_clock = now;
        if (packets::pending::session_kind == 1)
            pending::query_heartbeat(nullptr);
        else
            pending::query_lan_think(nullptr);
    }
    if (awaiting_heartbeat && heartbeat_elapsed > 30000) {
        awaiting_heartbeat = 0;
        packets::ScopedString text;
        gui::string_reference(&text.value, 1547);
        if (system(96)) {
            application::show_cursor(1);
            const Byte previous = game::runtime::reset_requested;
            game::runtime::reset_requested = 1;
            application::pending::MessageBox box;
            application::pending::construct_message_box(&box, &text.value);
            try {
                application::pending::run_message_box(&box, nullptr, 0);
            } catch (...) {
                application::pending::destroy_message_box(&box);
                throw;
            }
            game::runtime::reset_requested = previous;
            application::pending::destroy_message_box(&box);
        }
    }
}
namespace pending {

const char *server_race_name(void *settings) {
    if (at<Dword>(settings, 680) == 0)
        return "Single Race";
    if (at<Dword>(settings, 680) != 1)
        return "";
    return "Lap Knockout";
}

void __cdecl server_query_players(char *output, int, Dword) {
    void *settings = services::pending::server_settings;
    awaiting_heartbeat = 0;
    at<Dword>(settings, 664) = queue::count(player_queue());
    char hostname[100]{}, port[100]{}, map[100]{}, type[100]{}, players[100]{}, maximum[100]{},
        mode[100]{}, bandwidth[100]{}, password[100]{}, version[100]{}, catalogue[100]{};
    std::sprintf(hostname, "%s", reinterpret_cast<char *>(settings));
    std::sprintf(port, "%d", at<Dword>(settings, 656));
    std::sprintf(map, "%s", reinterpret_cast<char *>(settings) + 324);
    std::sprintf(type, "%s", server_race_name(settings));
    std::sprintf(players, "%d", at<Dword>(settings, 664));
    std::sprintf(maximum, "%d", at<Dword>(settings, 668));
    std::sprintf(mode, "%s", reinterpret_cast<char *>(settings) + 532);
    std::sprintf(bandwidth, "%d", at<Dword>(settings, 696));
    std::sprintf(password, "%d", at<Byte>(settings, 693));
    std::sprintf(version, "%d", 18088178);
    std::sprintf(catalogue, "%d", at<Dword>(settings, 528));
    std::sprintf(output,
                 "\\hostname\\%s\\hostport\\%s\\mapname\\%s\\gametype\\%s\\numplayers\\%"
                 "s\\maxplayers\\%s\\gamemode\\%s\\pbmd\\%s\\password\\%s\\nver\\%s\\ctid\\%s",
                 hostname, port, map, type, players, maximum, mode, bandwidth, password, version,
                 catalogue);
}

void __cdecl server_query_parameters(char *output, int, Dword) {
    char name[100]{}, version[100]{};
    awaiting_heartbeat = 0;
    std::sprintf(name, "%s", "nfs6");
    std::sprintf(version, "%s", version_text);
    std::sprintf(output, "\\gamename\\%s\\gamever\\%s", name, version);
}

void __cdecl server_query_rules(char *output, int, Dword) {
    void *settings = services::pending::server_settings;
    char resolution[100]{}, direction[100]{}, laps[100]{}, dedicated[100]{}, extra[100]{};
    awaiting_heartbeat = 0;
    std::sprintf(resolution, "%d", at<Dword>(settings, 688));
    std::sprintf(direction, "%d", at<Dword>(settings, 684));
    std::sprintf(laps, "%d", at<Dword>(settings, 672));
    std::sprintf(dedicated, "%d", at<Byte>(settings, 692));
    std::sprintf(extra, "%d", at<Byte>(settings, 694));
    std::sprintf(output, "\\res\\%s\\dir\\%s\\laps\\%s\\ded\\%s", resolution, direction, laps,
                 dedicated);
}

void __cdecl server_query_echo(char *output, int, Dword) {
    awaiting_heartbeat = 0;
    std::sprintf(output, "");
}
} // namespace pending
} // namespace nfs::network::session
