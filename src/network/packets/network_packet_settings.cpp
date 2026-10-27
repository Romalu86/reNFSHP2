#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/frame/game_step.hpp"
#include <cstring>
namespace nfs::network::packets {

Dword string_character(const gui::String *text, int index) {
    if (index >= 0 && index < text->length) {
        const Dword wide = text->wide;
        const void *data = gui::string_data(text);
        if (data)
            return wide ? static_cast<const std::uint16_t *>(data)[index]
                              : Dword(static_cast<const signed char *>(data)[index]);
    }
    return 0;
}

void export_server_settings(Dword *owner, void *settings) {
    auto *rules = system(72);
    at<Dword>(settings, 24) = at<Dword>(rules, 188);
    at<Dword>(settings, 28) = at<Dword>(rules, 184);
    at<Dword>(settings, 0) = at<Dword>(rules, 152);
    at<Dword>(settings, 4) = at<Dword>(rules, 164);
    for (unsigned index = 5; index <= 10; ++index)
        at<Dword>(settings, (index + 4) * 4) = rule_value(rules, index);
    at<Dword>(settings, 12) = game::race_ticks;
    at<Dword>(settings, 60) = at<Dword>(rules, 172);
    at<Dword>(settings, 16) = at<Dword>(rules, 168);
    at<Byte>(settings, 8) = Byte(queue::count(player_queue()));
    at<Dword>(settings, 20) = players::selected_players(player_queue());
    at<Dword>(settings, 32) = at<Dword>(owner, 136);
    at<Byte>(settings, 9) = at<Byte>(owner, 96);
    at<Dword>(network_game(), 48) = at<Dword>(owner, 136);
}

void import_server_settings(Dword *owner, void *settings) {
    auto *rules = system(72);
    pending::set_game_rule(rules, 7, at<Dword>(settings, 44));
    pending::set_rule_variant(rules, 7, at<Dword>(settings, 44));
    pending::set_rule_variant(rules, 2, at<Dword>(settings, 0));
    pending::set_game_rule(rules, 15, at<Dword>(settings, 24));
    pending::set_game_rule(rules, 3, at<Dword>(settings, 4));
    pending::set_game_rule(rules, 6, at<Dword>(settings, 40));
    pending::set_game_rule(rules, 5, at<Dword>(settings, 36));
    if (at<std::int16_t>(owner, 380) != 7)
        pending::select_track(system(40), at<Dword>(settings, 36));
    for (unsigned index = 8; index <= 10; ++index)
        pending::set_game_rule(rules, index, at<Dword>(settings, (index + 4) * 4));
    pending::set_game_rule(rules, 11, 0);
    pending::set_game_rule(rules, 14, 0);
    pending::set_game_rule(rules, 12, 0);
    at<Dword>(rules, 172) = at<Dword>(settings, 60);
    at<Dword>(rules, 184) = at<Dword>(settings, 28);
    at<Byte>(network_game(), 44) = at<Byte>(settings, 16);
    at<Dword>(network_game(), 48) = at<Dword>(settings, 32);
    at<Dword>(rules, 180) = 0;
    at<Dword>(owner, 2100) = at<Dword>(settings, 12);
}

bool find_player_route(Dword *owner, Dword identifier, void *destination) {
    ui::PlayerRecord player{};
    return copy_player(&identifier, &player) &&
           queue::copy_matching(route_queue(owner), address_of(bytes(&player) + 2),
                                address_of(destination), nullptr);
}
} // namespace nfs::network::packets
