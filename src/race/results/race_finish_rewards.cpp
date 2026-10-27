#include "../session/race_session.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::application::pending {
void complete_unfinished_results(void *);
void persist_race_statistics(void *);

Dword finish_position_reward(void *rewards, Dword position, int difficulty) {
    if (int(position) > 24 || position >= 24)
        return 0;
    if (difficulty < 0)
        difficulty = 0;
    else if (difficulty > 2)
        difficulty = 2;
    return at<Dword>(rewards, 12 + (position + Dword(difficulty) * 24) * 4);
}

Dword event_medal_reward(void *rewards, int medal) {
    if (!(services::special_race_mode(system(72)) & 255) || !system(72))
        return 0;
    if (medal < 0)
        medal = 0;
    else if (medal > 2)
        medal = 2;
    return at<Dword>(rewards, Dword(medal) * 4);
}

Dword improved_medal_reward(void *rewards, int previous, int medal) {
    if (!(services::special_race_mode(system(72)) & 255))
        return 0;
    if (previous < 0)
        previous = 0;
    else if (previous > 2)
        previous = 2;
    if (medal < 0)
        medal = 0;
    else if (medal > 2)
        medal = 2;
    if (medal <= previous)
        return 0;
    return at<Dword>(rewards, Dword(medal) * 4) - at<Dword>(rewards, Dword(previous) * 4);
}

void *profile_event_record(void *profile, Dword mode, Dword event) {
    if (mode == 2)
        return bytes(profile) + 8180 + event * 624;
    if (mode == 4)
        return bytes(profile) + 28772 + event * 624;
    return nullptr;
}

Byte profile_event_medal(void *record, int *medal) {
    switch (at<Dword>(record, 0)) {
    case 3:
        *medal = 0;
        return 1;
    case 4:
        *medal = 1;
        return 1;
    case 5:
        *medal = 2;
        return 1;
    default:
        return 0;
    }
}

int event_participant_count(void *progress) {
    const Dword flags = at<Dword>(progress, 0);
    return int(((flags >> 9) & 7) + int((flags & 256) != 0) + 1 + ((flags >> 12) & 15));
}

void record_event_finish_time(void *progress, Dword slot, std::uint16_t ticks) {
    at<std::uint16_t>(progress, 500 + slot * 2) = ticks;
}

void record_event_best_time(void *progress, Dword slot, std::uint16_t ticks) {
    at<std::uint16_t>(progress, 548 + slot * 2) = ticks;
}
// Independent entry;  inlines this loop.
void record_event_times(void *race) {
    void *rules = system(72);
    if (at<Dword>(rules, 152) == 2) {
        void *progress = active_event_progress(rules);
        const int lap = last_configured_lap(rules);
        for (int slot = 0; slot < event_participant_count(progress); ++slot) {
            auto *record = participant(race, Dword(slot));
            if (record->kind != 3 && record->kind != 2) {
                record_event_best_time(
                    progress, Dword(slot),
                    at<std::uint16_t>(record, offsetof(RaceParticipant, best_lap_ticks)));
                record_event_finish_time(
                    progress, Dword(slot),
                    std::uint16_t(lap < 0 ? -1 : record->checkpoint_times[lap]));
            }
        }
    }
}

void update_finish_rewards(void *race) {
    complete_unfinished_results(race);
    invoke(race, 5);
    void *rules = system(72);
    if (!rules)
        return;
    if (!at<Dword>(rules, 140)) {
        const int difficulty = at<int>(rules, 192);
        for (int slot = 0; slot < at<int>(system(72), 168); ++slot) {
            const int position = at<int>(race, 34296 + Dword(slot) * 4);
            if (position >= 120 || position < -2)
                continue;
            auto *record = participant(race, Dword(slot));
            if (record->kind != 3 && record->kind != 2 && at<int>(system(72), 180) != 0 &&
                !game::runtime::physics::split_screen() && at<Dword>(system(72), 148) != 5 &&
                !network::session_active() && invoke<Byte>(race, 35) &&
                int(record->arrests) <= at<int>(system(72), 2264))
                add_participant_reward(
                    record, 0,
                    finish_position_reward(bytes(race) + 164, Dword(position), difficulty));
            const int result = at<int>(race, 34392 + Dword(slot) * 4);
            if (slot != 0 || !(services::special_race_mode(system(72)) & 255) || result < 0 ||
                result > 2)
                continue;
            if (at<Dword>(rules, 152) == 2 &&
                !event_complete_progress(active_event_progress(rules)))
                continue;
            const int medal = 2 - result;
            void *profile = bytes(rules) + 2488;
            if (!profile)
                continue;
            void *event =
                profile_event_record(profile, at<Dword>(rules, 148), at<Dword>(rules, 102920));
            const Dword multiplier = Dword(at<Dword>(rules, 152) == 9) + 1;
            int previous;
            const Dword reward = profile_event_medal(event, &previous)
                                    ? improved_medal_reward(bytes(race) + 164, previous, medal)
                                    : event_medal_reward(bytes(race) + 164, medal);
            add_participant_reward(record, 3, reward * multiplier);
            if (reward)
                record->awarded_medal = medal;
            if (at<int>(event, 0) < medal + 3)
                at<int>(event, 0) = medal + 3;
        }
    }
    persist_race_statistics(race);
    rules = system(72);
    if (at<Dword>(rules, 152) == 2) {
        void *progress = active_event_progress(rules);
        const int lap = last_configured_lap(rules);
        for (int slot = 0; slot < event_participant_count(progress); ++slot) {
            auto *record = participant(race, Dword(slot));
            if (record->kind != 3 && record->kind != 2) {
                record_event_best_time(
                    progress, Dword(slot),
                    at<std::uint16_t>(record, offsetof(RaceParticipant, best_lap_ticks)));
                record_event_finish_time(
                    progress, Dword(slot),
                    std::uint16_t(lap < 0 ? -1 : record->checkpoint_times[lap]));
            }
        }
    }
    invoke(race, 47);
}
} // namespace nfs::application::pending
