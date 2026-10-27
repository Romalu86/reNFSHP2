#include <winsock2.h>
#include "frontend_network_session.hpp"
#include "../../network/session/network_lifecycle_runtime.hpp"
#include "../../vehicles/rendering/vehicle_renderable_resources.hpp"
#include "../../network/session/server_runtime.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::network::session::pending {
bool local_route(std::uint16_t);  
}
namespace nfs::network::packets::pending {
bool is_banned(const char *);                               
void record_ban(const char *, const char *, const char *);
} // namespace nfs::network::packets::pending
namespace nfs::application::pending {
void send_race_text(void *, Dword, Dword, Dword, Dword, Dword, Dword);  
}
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
using game::runtime::system;
namespace queue = lifecycle::construction::network::queue;
namespace {
// Native buffer at EBP-154 borders the player record at EBP-E0. The initial
// 119-byte copy includes the first three record bytes; later record writes are retained.
struct CommandScratch {
    char text[116];
    network::ui::PlayerRecord player;
};
static_assert(offsetof(CommandScratch, player) == 116);
bool past_limit(const char *limit, const char *position) {
    return reinterpret_cast<Dword>(limit) < reinterpret_cast<Dword>(position);
}
int command_track(const char *name) { // Shared network command handling.
    pending::LocalFrontendString label;
    gui::string_reference(&label.value, 2613);
    gui::string_make_narrow(&label.value);
    if (!_strnicmp(name, gui::string_ascii(&label.value), 5))
        return 12;
    int selected = -1;
    for (int index = 0; index < 12; ++index) {
        gui::string_reference(&label.value, network::packets::track_reference(index));
        gui::string_make_narrow(&label.value);
        if (!_strnicmp(name, gui::string_ascii(&label.value), 5))
            selected = index;
    }
    if (selected < 0) {
        const int number = std::atoi(name);
        if (number > 0)
            selected = number - 1;
    }
    return selected;
}
bool restricted_knockout_track(int track) {
    return at<int>(system(72), 152) == 1 &&
           (track == 5 || track == 11 || track == 8 || track == 2);
}
void command_help(NetworkSessionWidget *widget) {
    pending::LocalFrontendString text(&lifecycle::construction::unresolved::shared_string);
    auto line = [&](const char *value) {
        gui::string_assign(&text.value, value);
        session_notice(widget, &text.value, -1, 0, game::runtime::reset_complete);
    };
    if (at<Dword>(network::client(), 68)) {
        for (const char *value : {"/request Car", "/name Playername", "/voteboot Playername",
                                  "/votebootbyid PlayerID", "/votemap TrackName", "/setup"})
            line(value);
    }
    for (const char *value :
         {"/send Playername,Message", "/quit", "/disconnect", "/challenge", "/reset", "/idlist"})
        line(value);
    if (at<Dword>(network::server(), 44)) {
        for (const char *value : {"/startrace",
                                  "/server finishtimeout Milliseconds",
                                  "/server laptimeout Milliseconds",
                                  "/server maxsetuptime Milliseconds",
                                  "/server heartbeat Milliseconds",
                                  "/server resultsdisplaytime Milliseconds",
                                  "/server countdown Milliseconds",
                                  "/server docountdown 0|1",
                                  "/server voting 0|1",
                                  "/server numraces NumRaces",
                                  "/server laps NumLaps",
                                  "/server class 0|1|2|3|4|5",
                                  "/server direction 0|1|2|3|4",
                                  "/track TrackName",
                                  "/racetype ko|sr",
                                  "/servername NewServerName",
                                  "/password NewPassword",
                                  "/ban Playername",
                                  "/boot Playername",
                                  "/banbyid PlayerID",
                                  "/bootbyid PlayerID"})
            line(value);
    }
    line("/? for command list");
}
} // namespace

Dword execute_network_command(NetworkSessionWidget *widget, const char *input) {
    if (!input)
        return 0;
    const Dword length = Dword(std::strlen(input));
    if (!length)
        return 0;
    CommandScratch scratch;
    char *buffer = reinterpret_cast<char *>(&scratch);
    char *limit = reinterpret_cast<char *>(reinterpret_cast<Dword>(buffer) + length);
    std::strncpy(buffer, input, 119);
    if (*buffer != '/')
        return 0;
    char *command = std::strtok(buffer + 1, " ");
    if (!command)
        return 1;
    auto matches = [&](const char *name) { return !_stricmp(command, name); };
    auto remainder = [&] { return command + std::strlen(command) + 1; };
    auto report = [&](Dword reference) {
        pending::LocalFrontendString text;
        gui::string_reference(&text.value, reference);
        session_notice(widget, &text.value);
    };
    int consumed;
    auto argument = [&]() {
        return parse_network_argument(remainder(), &consumed, nullptr, " ", nullptr);
    };
    auto find_name = [&](const char *name) {
        return network::packets::copy_player(name, &scratch.player,
                                             &network::packets::pending::command_name_comparer);
    };
    if (matches("req") || matches("request")) {
        if (past_limit(limit, remainder()))
            return 1;
        char *name = argument();
        if (!name) {
            report(1645);
            return 1;
        }
        network::ui::CarSettings settings{};
        const int car =
            int(game::world::vehicle_render::catalogue_index(bytes(system(52)) + 4, name));
        at<int>(&settings, 60) = car;
        if (car == -1) {
            report(1646);
            return 1;
        }
        if (network::packets::vehicle_class(reinterpret_cast<Dword *>(bytes(system(52)) + 4),
                                            car) <
            int(network::packets::rule_value(system(72), 15))) {
            report(1647);
            return 1;
        }
        at<Dword>(&settings, 0) = at<Dword>(&settings, 68) = 1;
        at<Dword>(&settings, 8) = at<Dword>(&settings, 16) = 0;
        pending::LocalFrontendString text, wide_name(name);
        gui::string_make_wide(&wide_name.value);
        const Dword success = network::request_client_car(network::client(), &settings);
        {
            pending::LocalFrontendString pattern;
            gui::string_reference(&pattern.value, success ? 1613 : 1614);
            gui::string_format(&text.value, &pattern.value, gui::string_data(&wide_name.value));
        }
        session_notice(widget, &text.value);
        return 1;
    }
    if (matches("name")) {
        if (past_limit(limit, remainder()) || !at<Dword>(network::client(), 68))
            return 1;
        const char *value = argument();
        if (!value) {
            report(1648);
            return 1;
        }
        char name[13];
        std::strncpy(name, value, 12);
        name[12] = 0;
        network::ui::initialize_car_settings(&scratch.player.car);
        if (find_name(name))
            report(1649);
        else
            network::send_client_rename(network::client(), name);
        return 1;
    }
    if (matches("voteboot") || matches("votebootbyid")) {
        const bool by_id = matches("votebootbyid");
        if (past_limit(limit, remainder()))
            return 1;
        char *value = argument();
        if (!value) {
            report(1650);
            return 1;
        }
        network::ui::initialize_car_settings(&scratch.player.car);
        bool found;
        if (by_id) {
            int identifier = std::atoi(value);
            found = network::packets::copy_player(&identifier, &scratch.player);
        } else
            found = find_name(value);
        if (!found)
            report(1651);
        else
            network::send_client_vote(network::client(), 2,
                                      Dword(std::int16_t(scratch.player.identifier)));
        return 1;
    }
    if (matches("votemap")) {
        if (past_limit(limit, remainder()))
            return 1;
        char *name = argument();
        if (!name) {
            report(1652);
            return 1;
        }
        const int track = command_track(name);
        if (track < 0 || track > 12) {
            report(1653);
            return 1;
        }
        if (!restricted_knockout_track(track))
            network::send_client_vote(network::client(), 3, Dword(track));
        return 1;
    }
    if (matches("server")) {
        if (!at<Dword>(network::server(), 44) || past_limit(limit, remainder()))
            return 1;
        execute_server_setting(widget, remainder(), limit);
        return 1;
    }
    if (matches("startrace")) {
        if (at<Dword>(network::server(), 44) &&
            at<std::int16_t>(network::network_game(), 8) == 2) {
            pending::send_race_text(network::server(), 1654, Dword(-2), 0, 0, 0, 0);
            network::session::begin_server_load(network::server());
        }
        return 1;
    }
    if (matches("servername") || matches("password") || matches("racetype")) {
        if (past_limit(limit, remainder()) || !at<Dword>(network::server(), 44))
            return 1;
        char *value = argument();
        if (!value) {
            report(1642);
            return 1;
        }
        if (matches("servername")) {
            auto *server = network::server();
            auto *name = reinterpret_cast<char *>(server) + 172;
            std::strncpy(name, value, 24);
            at<std::uint8_t>(server, 195) = 0;
            network::packets::set_server_name(network::network_game(), name);
        } else if (matches("password")) {
            auto *server = network::server();
            at<std::uint8_t>(server, 208) = 0;
            std::strncpy(reinterpret_cast<char *>(server) + 196, value, 24);
        } else {
            if (!_stricmp(value, "ko"))
                at<Dword>(network::server(), 56) = 1;
            else if (!_stricmp(value, "sr"))
                at<Dword>(network::server(), 56) = 0;
        }
        return 1;
    }
    if (matches("track")) {
        if (past_limit(limit, remainder()) || !at<Dword>(network::server(), 44))
            return 1;
        char *name = argument();
        if (!name) {
            report(1652);
            return 1;
        }
        const int track = command_track(name);
        if (track < 0 || track > 12) {
            report(1653);
            return 1;
        }
        auto *server = network::server();
        if (restricted_knockout_track(track))
            at<Dword>(server, 56) = 0;
        at<int>(server, 64) = track;
        return 1;
    }
    if (matches("idlist")) {
        network::ui::initialize_car_settings(&scratch.player.car);
        pending::LocalFrontendString text(
            &lifecycle::construction::unresolved::shared_string);
        for (int row = 0; row < int(queue::count(network::packets::player_queue())); ++row)
            if (queue::copy_nth(network::packets::player_queue(), 0, row, 0,
                                reinterpret_cast<Dword>(&scratch.player), nullptr, 0, nullptr)) {
                gui::string_format_ascii(&text.value, "%s : %d", scratch.player.name,
                                         int(std::int16_t(scratch.player.identifier)));
                session_notice(widget, &text.value);
            }
        return 1;
    }
    if (matches("bootbyid") || matches("banbyid") || matches("boot") || matches("ban")) {
        const bool by_id = matches("bootbyid") || matches("banbyid");
        const bool ban = matches("ban") || matches("banbyid");
        if (past_limit(limit, remainder()) || !at<Dword>(network::server(), 44))
            return 1;
        const char *value = argument();
        if (!value) {
            report(ban ? 1658 : 1655);
            return 1;
        }
        network::ui::initialize_car_settings(&scratch.player.car);
        bool found;
        if (by_id) {
            int identifier = std::atoi(value);
            found = network::packets::copy_player(&identifier, &scratch.player);
        } else
            found = find_name(value);
        if (!found) {
            report(ban ? 1659 : 1656);
            return 1;
        }
        const int identifier = std::int16_t(scratch.player.identifier);
        const auto route_id = at<std::uint16_t>(&scratch.player, 2);
        alignas(4) std::uint8_t route[304];
        if (ban) {
            if (network::session::pending::local_route(route_id)) {
                report(1660);
                return 1;
            }
        } else {
            if (!queue::copy_matching(network::session::route_queue(),
                                      reinterpret_cast<Dword>(&route_id),
                                      reinterpret_cast<Dword>(route), nullptr)) {
                report(1657);
                return 1;
            }
            if (at<Dword>(route, 4)) {
                report(1718);
                return 1;
            }
        }
        auto *authentication =
            network::packets::pending::authentication_record(identifier);
        bool announce_ban = false;
        if (authentication) {
            auto *key = reinterpret_cast<char *>(authentication) + 325;
            if (std::strlen(key)) {
                announce_ban = true;
                in_addr address;
                address.s_addr =
                    network::authentication::player_address(network::server(), identifier);
                if (ban) {
                    if (!network::packets::pending::is_banned(key))
                        network::packets::pending::record_ban(key, scratch.player.name,
                                                              inet_ntoa(address));
                } else if (!network::packets::pending::is_booted(key))
                    network::packets::pending::record_boot(key, scratch.player.name,
                                                                    inet_ntoa(address));
            }
        }
        if (ban) {
            if (announce_ban)
                report(1663);
            network::authentication::reject_player(network::server(), Dword(identifier), 6);
        } else {
            while (network::remove_server_client(network::server(), nullptr, at<Dword>(route, 0))) {
            }
            network::session::boot_route(route, 14);
            report(1664);
        }
        return 1;
    }
    if (matches("setup")) {
        if (at<std::int16_t>(network::network_game(), 8) == 2) {
            gui::events::propagate(widget, 111, 0, 0, 2);
            game::phase_reset = 0;
        }
        return 1;
    }
    if (matches("send") || matches("whisper")) {
        if (past_limit(limit, remainder()))
            return 1;
        char *name = parse_network_argument(remainder(), &consumed, ",", " ", nullptr);
        if (!name) {
            report(1665);
            return 1;
        }
        network::ui::initialize_car_settings(&scratch.player.car);
        if (!find_name(name)) {
            report(1666);
            return 1;
        }
        char *message = name + consumed;
        if (past_limit(limit, message) || !*message) {
            report(1667);
            return 1;
        }
        if (at<Dword>(network::client(), 68))
            network::send_client_whisper(network::client(), message,
                                         std::uint16_t(scratch.player.identifier));
        else if (at<Dword>(network::server(), 44)) {
            alignas(4) std::uint8_t route[304];
            auto route_id = at<std::uint16_t>(&scratch.player, 2);
            if (queue::copy_matching(network::session::route_queue(),
                                     reinterpret_cast<Dword>(&route_id),
                                     reinterpret_cast<Dword>(route), nullptr))
                network::packets::broadcast_chat(network::server(), message, route);
            else
                report(1657);
        }
        return 1;
    }
    if (matches("quit") || matches("exit")) {
        if (at<Dword>(network::client(), 68))
            network::disconnect_client(network::client(), 16);
        if (at<Dword>(network::server(), 44))
            network::leave_game(network::server());
        if (auto *root = pending::widget_root(&widget->base.base))
            family::finish_modal(root, 6, 0);
        return 1;
    }
    if (matches("disconnect")) {
        if (at<Dword>(network::client(), 68))
            network::disconnect_client(network::client(), 16);
        if (at<Dword>(network::server(), 44))
            network::leave_game(network::server());
        at<Dword>(system(96), 36) = 19;
        return 1;
    }
    if (matches("challenge") || matches("reset")) {
        if (at<Dword>(network::server(), 44)) {
            pending::send_race_text(network::server(), 1637, Dword(-2), 0, 0, 0, 0);
            pending::rotate_server_race(network::server());
        } else if (at<Dword>(network::client(), 68) &&
                   at<std::int16_t>(network::network_game(), 8) >= 5)
            network::session::request_spectator_mode(network::client());
        return 1;
    }
    if (matches("?") || matches("commands"))
        command_help(widget);
    else
        report(1669);
    return 1;
}
} // namespace nfs::application::frontend
