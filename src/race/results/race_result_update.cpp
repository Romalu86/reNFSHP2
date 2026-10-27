#include "../session/race_session.hpp"
#include <cstring>
namespace nfs::application::pending {

void __fastcall update_race_results(void *race, void *) {
    void *rules = system(72);
    auto *positions = &at<int>(race, 34296);
    auto *results = &at<int>(race, 34392);
    for (unsigned slot = 0; slot < 24; ++slot) {
        positions[slot] = 72;
        results[slot] = 72;
    }
    switch (at<Dword>(rules, 152)) {
    case 1:
        rank_elimination_finishers(race);
        break;
    case 2: {
        rank_race_racers(race);
        void *progress = active_event_progress(rules);
        const Dword stage = event_stage(progress);
        for (int slot = 0; slot < at<int>(rules, 168); ++slot) {
            auto *record = participant(race, Dword(slot));
            if (record->kind != 3 && record->kind != 2) {
                const int lap = last_configured_lap(rules);
                const int position = lap < 0 ? -1 : record->checkpoint_positions[lap];
                record_event_position(progress, stage, Dword(slot), Byte(position));
            }
        }
        advance_event_progress(progress);
        if (!event_complete_progress(progress)) {
            std::memcpy(results, positions, 24 * 4);
            return;
        }
        int current_positions[24];
        std::memcpy(current_positions, positions, sizeof(current_positions));
        rank_race_finishers(race);
        std::memcpy(results, positions, 24 * 4);
        std::memcpy(positions, current_positions, sizeof(current_positions));
        goto evaluate_medal;
    }
    case 4:
    case 9:
    case 10:
    case 11:
    case 13:
        rank_race_cops(race);
        break;
    case 6:
        rank_quota_players(race);
        break;
    default:
        if (!invoke<Byte>(race, 9))
            rank_race_racers(race);
        else
            rank_race_cops(race);
    }
    std::memcpy(results, positions, 24 * 4);
evaluate_medal:
    if (services::special_race_mode(rules) & 255) {
        int thresholds[18];
        Dword additional[3];
        read_race_reward_rules(rules, thresholds, additional);
        const int kind = participant(race, 0)->kind;
        const int medal =
            invoke<int>(race, 6, thresholds, participant(race, 0), Byte(kind == 3 || kind == 2));
        results[0] = medal == -1 ? 72 : 2 - medal;
    }
}
} // namespace nfs::application::pending
