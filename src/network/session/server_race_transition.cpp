#include "server_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/gui/events/gui_events.hpp"
namespace nfs::application::pending {
void create_network_camera(void *);
void set_network_camera_overlay(void *, Byte);
void reload_catalogue_renderables(void *);

void load_network_race(void *game_owner) {
    if (at<Dword>(game_owner, 32)) {
        at<Dword>(game_owner, 32) = at<Dword>(game_owner, 16) = 0;
        return;
    }
    Dword *rules = system(72);
    at<Dword>(game_owner, 48) = 0;
    at<Byte>(game_owner, 44) = 0;
    at<Dword>(rules, 168) = at<Dword>(rules, 172) = 0;
    at<Dword>(rules, 168) = at<Dword>(rules, 172) = at<Dword>(rules, 184) = 0;
    network::packets::pending::set_game_rule(rules, 14, 0);
    network::ui::CarSettings empty;
    network::ui::initialize_car_settings(&empty);
    for (unsigned word = 0; word < 21; ++word)
        at<Dword>(&empty, word * 4) = 0;
    for (Dword slot = 0; slot < 24; ++slot)
        for (unsigned word = 0; word < 21; ++word)
            at<Dword>(grid_entry(rules, slot), word * 4) = at<Dword>(&empty, word * 4);
    lifecycle::construction::network::queue::visit(
        network::server_queue(static_cast<Dword *>(game_owner)), 0, 0,
        reinterpret_cast<Dword *>(&network::players::prepare), 0, nullptr);
    if (at<std::int16_t>(game_owner, 8) == 3)
        at<std::int16_t>(game_owner, 8) = 4;
    at<std::int16_t>(game_owner, 10) = at<std::int16_t>(game_owner, 8);
    invoke(system(68), 3);
    invoke(system(76), 3);
    invoke(system(68), 5);
    invoke(system(76), 5);
    invoke(system(84), 5);
    for (int player = 0; player < at<int>(system(72), 176); ++player)
        game::world::bind_camera_player(game::tick_services[player], player);
    game::audio::load_race_banks_if_ready();
    if (!at<Dword>(system(108), 44) || !at<Dword>(system(108), 48))
        for (int player = 0; player < at<int>(system(72), 176); ++player) {
            game::world::pending::set_camera_mode(game::tick_services[player], 0);
            set_network_camera_overlay(game::tick_services[player], 0);
        }
    at<Dword>(game_owner, 80) = at<Dword>(game_owner, 16) = 0;
}

void unload_network_race(void *game_owner) {
    at<std::int16_t>(game_owner, 8) = at<std::int16_t>(game_owner, 10) = 2;
    invoke(at<void *>(system(92), 4), 17);
    at<Byte>(at<void *>(system(92), 4), 44) = 0;
    reset_network_clocks();
    frame_clock::elapsed = 0;
    frame_clock::step = 1;
    at<Dword>(system(72), 168) = at<Dword>(system(72), 172) = 0;
    lifecycle::construction::network::queue::visit(
        network::server_queue(static_cast<Dword *>(game_owner)), 0, 0,
        reinterpret_cast<Dword *>(&network::players::ready), 0, nullptr);
    game::audio::release_race_banks();
    invoke(system(36), 5);
    for (int player = 0; player < at<int>(system(72), 176); ++player) {
        void *cars = system(68), *camera = game::tick_services[player];
        at<Dword>(camera, 288) = at<Dword>(cars, 784);
        at<float>(camera, 292) = float(static_cast<long double>(at<float>(cars, 788)) + 2.0f);
        at<Dword>(camera, 296) = at<Dword>(cars, 792);
        if (!at<Dword>(system(108), 44) || !at<Dword>(system(108), 48)) {
            invoke(game::tick_services[player], 4);
            create_network_camera(game::tick_services[player]);
            invoke(game::tick_services[player], 3);
            set_network_camera_overlay(game::tick_services[player], 1);
        }
        at<Dword>(game::tick_services[player], 300) = 0;
        if (unsigned(player) < 2)
            at<Dword>(system(72), 204 + Dword(player) * 4) = Dword(-1);
    }
    invoke(system(76), 4);
    invoke(system(68), 4);
    reload_catalogue_renderables(system(52));
    at<Dword>(game_owner, 48) = 0;
    at<Byte>(game_owner, 44) = 0;
    lifecycle::phase_dependencies::finalize_vehicle_load();
    invoke(system(116), 5);
    at<Dword>(game_owner, 32) = at<Dword>(game_owner, 20) = 0;
}
} // namespace nfs::application::pending
namespace nfs::services::pending {

void transition_network(void *game_owner) {
    using namespace application::pending;
    if (at<Dword>(game_owner, 24)) {
        at<Dword>(game_owner, 24) = 0;
        lifecycle::construction::network::queue::visit(
            network::server_queue(static_cast<Dword *>(game_owner)), 0, 0,
            reinterpret_cast<Dword *>(&network::players::ready), 0, nullptr);
        at<Dword>(system(96), 36) = 24;
        at<std::int16_t>(game_owner, 10) = at<std::int16_t>(game_owner, 8);
        return;
    }
    if (at<Dword>(game_owner, 28)) {
        if (!at<Dword>(game_owner, 20)) {
            invoke(at<void *>(system(92), 4), 3, Dword{0});
            invoke(at<void *>(system(92), 4), 2);
            gui::events::propagate(at<void *>(system(96), 40), 112, 0, 0, 2);
        }
        at<Dword>(game_owner, 28) = 0;
    }
    if (at<Dword>(game_owner, 20)) {
        unload_network_race(game_owner);
    }
    if (at<Dword>(game_owner, 16)) {
        load_network_race(game_owner);
    }
    at<std::int16_t>(game_owner, 10) = at<std::int16_t>(game_owner, 8);
}
} // namespace nfs::services::pending
