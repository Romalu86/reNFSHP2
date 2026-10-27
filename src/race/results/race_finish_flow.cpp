#include "../session/race_session.hpp"
#include "nfs/audio/music/music_clients.hpp"
namespace nfs::application::pending {
void finish_network_race(void *);

void save_race_snapshot(void *manager, void *race) {
    if (void *snapshot = at<void *>(manager, 8))
        invoke(snapshot, 33, Dword(1));
    at<void *>(manager, 8) = invoke<void *>(race, 34);
}

void request_network_finish() {
    if (at<Dword>(system(108), 44))
        finish_network_race(system(108));
}

void __fastcall finish_race_run(void *race, void *) {
    const int deadline = at<int>(race, 34596);
    if (deadline < 0 || int(race_clock()) < deadline) {
        invoke(race, 3, Dword(0));
        return;
    }
    for (Dword slot = 0; slot < 24; ++slot)
        capture_participant_finish(race, slot);
    if (at<Dword>(system(72), 140)) {
        at<Byte>(race, 44) = 1;
        at<Dword>(system(96), 36) = 23;
        return;
    }
    if (at<Byte>(race, 44))
        return;
    at<Byte>(race, 44) = 1;
    at<Dword>(race, 34600) = invoke<Dword>(race, 22);
    ++at<Dword>(race, 34608);
    save_race_snapshot(system(92), race);
    update_finish_rewards(at<void *>(system(92), 8));
    if (network::session_active()) {
        request_network_finish();
        return;
    }
    at<Dword>(system(96), 36) = at<Dword>(race, 34620) == 6 ? 28 : 21;
}

void __fastcall display_race_results(void *race, void *) {
    void *rules = system(72);
    int places[2]; // A slot of -1 leaves its saved stack word unwritten.
    for (unsigned player = 0; player < 2; ++player) {
        const int slot = at<int>(rules, 204 + player * 4);
        if (slot != -1) {
            places[player] = at<int>(race, 34296 + Dword(slot) * 4);
            if (int(participant(race, Dword(slot))->arrests) > at<int>(rules, 2264))
                places[player] = 72;
        }
    }
    if (!(services::special_race_mode(system(72)) & 255))
        announce_race_results(system(84), int(Dword(places[0]) + 1), int(Dword(places[1]) + 1));
    game::audio::stop_race_channels();
}
} // namespace nfs::application::pending
