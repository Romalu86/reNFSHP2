#include "../session/server_runtime.hpp"
#include "../../frontend/flow/frontend_runtime.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::network::packets::pending {
extern int session_kind;
}
namespace nfs::application::pending {

void reset_race_round(void *race) {
    at<Byte>(race, 44) = 0;
    invoke(race, 17);
}
// Keep the rotation writes in this order.
void reset_network_clocks() {
    game::stop_requested = 0;
    graphics::unresolved::camera_state = 0;
    game::race_ticks = 0;
    game::runtime::started_frames = 0;
    frame_clock::paused = 0;
    game::count_race_ticks = 0;
    game::runtime::race_started = 0;
    game::start_deadline = 0;
}

void clear_boot_list() {
    char filename[260]{};
    std::sprintf(filename, "%s%s", graphics::startup::configuration_path(), "bootlist.txt");
    if (FILE *file = std::fopen(filename, "w"))
        std::fclose(file);
}

void rotate_server_race(void *server) {
    void *settings = services::pending::server_settings;
    void *rules = system(72);
    Dword direction = at<Dword>(server, 68), weather = at<Dword>(server, 84);
    int track = at<int>(server, 64);
    const int round = at<int>(server, 124);
    const bool first_round = round == 0;
    at<Dword>(server, 124) = Dword(round) + 1;
    if (at<int>(server, 76) <= int(Dword(round) + 1))
        at<Dword>(server, 124) = 0;
    auto sprint = [](int id) { return id == 5 || id == 11 || id == 8 || id == 2; };
    if (first_round) {
        const int selected = next_server_track(settings);
        if (selected >= 0 && selected < 13) {
            at<int>(server, 64) = track = selected;
            if (sprint(track))
                at<Dword>(server, 56) = 0;
        }
    }
    if (track == 12) {
        track = at<int>(server, 116);
        if (first_round || (at<Dword>(server, 56) == 1 && sprint(track))) {
            do {
                Dword index = network::rules::random() % at<Dword>(system(40), 4);
                if (int(index) < 0 || index > 12)
                    index = 0;
                track = at<int>(at<void *>(system(40), 12 + index * 4), 20);
            } while (at<Dword>(server, 56) == 1 && sprint(track));
        }
    }
    auto set_rule = [&](int index, Dword value) {
        network::packets::pending::set_game_rule(static_cast<Dword *>(rules), index, value);
    };
    set_rule(5, Dword(track));
    network::packets::pending::select_track(system(40), Dword(track));
    const bool track_changed = at<int>(server, 116) != track;
    at<int>(server, 116) = track;
    if (direction == 4)
        direction = first_round ? network::rules::random() & 3 : at<Dword>(server, 120);
    set_rule(6, direction);
    const bool direction_changed = at<Dword>(server, 120) != direction;
    at<Dword>(server, 120) = direction;
    if (weather == 3)
        weather = network::rules::random() % 3;
    set_rule(10, weather);
    if (sprint(track)) {
        set_rule(7, 1);
        at<Dword>(server, 56) = 0;
    } else {
        int laps = at<int>(server, 72);
        if (laps >= 0)
            set_rule(7, Dword(laps));
        else if (first_round || at<int>(rules, 228) <= 0 || at<int>(rules, 228) >= 10)
            set_rule(7, (network::rules::random() & 3) * 2 + 2);
    }
    set_rule(8, at<Dword>(server, 76));
    set_rule(9, 0);
    const bool mode_changed =
        network::packets::rule_value(static_cast<Dword *>(rules), 2) != at<Dword>(server, 56);
    set_rule(2, at<Dword>(server, 56));
    set_rule(15, at<Dword>(server, 60));
    set_rule(3, at<Dword>(server, 92));
    set_rule(11, 0);
    set_rule(14, 0);
    set_rule(12, 0);
    at<Dword>(rules, 168) = 0;
    at<Dword>(rules, 172) = 0;
    at<Dword>(rules, 184) = 0;
    at<Dword>(server, 156) = 0;
    at<Dword>(server, 148) = 0;
    at<Dword>(server, 152) = 0;
    namespace q = lifecycle::construction::network::queue;
    q::clear(at<Dword *>(server, 312));
    if (at<std::int16_t>(server, 12) > 2) {
        if (!track_changed && !direction_changed && !mode_changed) {
            at<std::int16_t>(server, 12) = 3;
            network::set_game_phase(system(104), 3);
            services::pending::transition_network(system(104));
            at<Dword>(server, 28) = network::transport_time();
            at<Dword>(server, 32) = network::transport_time();
            at<Dword>(server, 36) = at<Dword>(server, 24) = 0;
        } else {
            at<std::int16_t>(server, 12) = 2;
            network::set_game_phase(system(104), 2);
            reset_race_round(at<void *>(system(92), 4));
            reset_race_round(at<void *>(system(92), 8));
            frame_clock::elapsed = 0;
            frame_clock::step = 1;
            at<Dword>(system(72), 168) = at<Dword>(system(72), 172) = 0;
            reset_network_clocks();
            q::visit(network::server_queue(system(104)), 0, 0,
                     reinterpret_cast<Dword *>(&clear_grid_visitor), 0, nullptr);
            network::ui::PlayerRecord player;
            network::ui::initialize_car_settings(&player.car);
            for (int index = 0; index < int(q::count(network::server_queue(system(104))));
                 ++index) {
                if (q::copy_nth(network::server_queue(system(104)), 0, index, 0,
                                reinterpret_cast<Dword>(&player), 0, 0, nullptr)) {
                    struct JoinPacket {
                        Byte opcode, padding[3];
                        network::ui::PlayerRecord player;
                    } packet;
                    network::ui::initialize_car_settings(&packet.player.car);
                    packet.opcode = 4;
                    for (unsigned word = 0; word < 34; ++word)
                        at<Dword>(&packet.player, word * 4) = at<Dword>(&player, word * 4);
                    network::broadcast(static_cast<Dword *>(server), &packet, sizeof(packet), 32,
                                       nullptr);
                }
            }
        }
        struct StatePacket {
            Byte opcode;
            char name[25];
            std::int16_t phase;
            Byte settings[64];
        } packet;
        static_assert(sizeof(StatePacket) == 92);
        network::packets::clear_server_settings(packet.settings + 36);
        packet.opcode = 2;
        network::packets::export_server_settings(static_cast<Dword *>(server), packet.settings);
        packet.phase = at<std::int16_t>(server, 12);
        std::strcpy(packet.name, reinterpret_cast<char *>(bytes(server) + 172));
        network::broadcast(static_cast<Dword *>(server), &packet, sizeof(packet), 32, nullptr);
    }
    std::strcpy(reinterpret_cast<char *>(bytes(settings) + 632),
                reinterpret_cast<char *>(bytes(server) + 196));
    at<Byte>(settings, 693) = at<Byte>(server, 196) != 0;
    at<Byte>(settings, 694) = server_password_enabled != 0;
    std::strcpy(reinterpret_cast<char *>(settings), reinterpret_cast<char *>(bytes(server) + 172));
    LocalFrontendString name;
    gui::string_reference(&name.value, network::packets::track_reference(track));
    gui::string_make_narrow(&name.value);
    std::strcpy(reinterpret_cast<char *>(bytes(settings) + 324), gui::string_ascii(&name.value));
    at<int>(settings, 528) = track;
    const Dword mode = at<Dword>(server, 56);
    services::set_server_race_setting(settings, Dword(mode == 1));
    set_server_car_class(settings, at<int>(server, 60));
    set_server_direction(settings, at<Dword>(server, 68));
    if (mode != 1)
        at<Dword>(settings, 672) = at<Dword>(server, 72);
    at<Dword>(settings, 656) = at<std::uint16_t>(network::transport, 36);
    std::strcpy(reinterpret_cast<char *>(bytes(settings) + 532), "openplaying");
    at<Dword>(settings, 696) = Dword(network::packets::pending::session_kind);
    save_server_settings(settings);
    if ((network::ui::console_available() & 255) &&
        (track_changed || direction_changed || mode_changed)) {
        LocalFrontendString line;
        gui::string_format_ascii(&line.value, "Mode: %s",
                                 reinterpret_cast<char *>(bytes(settings) + 532));
        network::ui::console_print(gui::string_ascii(&line.value));
        gui::string_format_ascii(&line.value, "Map: %s",
                                 reinterpret_cast<char *>(bytes(settings) + 324));
        network::ui::console_print(gui::string_ascii(&line.value));
    }
    clear_boot_list();
    at<Dword>(server, 240) = at<Dword>(server, 236) = 0;
}
} // namespace nfs::application::pending
