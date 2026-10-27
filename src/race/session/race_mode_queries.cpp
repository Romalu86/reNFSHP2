#include "race_runtime.hpp"
namespace nfs::application::pending {
namespace {
int next_option(int current, const int *options, int count) {
    int index = 0;
    while (index < count && options[index] != current)
        ++index;
    ++index;
    if (index >= count)
        index = 0;
    return options[index];
}
} // namespace

int __fastcall quota_next_option(void *, void *, int current) {
    const int options[] = {5, 6, 7};
    return next_option(current, options, 3);
}

int __fastcall timed_next_option(void *, void *, int current) {
    const int options[] = {0, 6, 7};
    return next_option(current, options, 3);
}

int __fastcall paired_next_option(void *, void *, int current) {
    const int options[] = {0, 6, 7};
    return next_option(current, options, 3);
}

int __fastcall pursuit_next_option(void *, void *, int current) {
    const int options[] = {3, 6, 7};
    return next_option(current, options, 3);
}

int __fastcall elimination_next_option(void *, void *, int current) {
    int options[8] = {1, 0};
    int count = 2;
    if (at<Dword>(system(72), 176) == 1)
        options[count++] = 4;
    options[count++] = 6;
    options[count++] = 7;
    return next_option(current, options, count);
}

int __fastcall time_trial_next_option(void *, void *, int current) {
    int options[8];
    int count = 0;
    if (at<int>(system(72), 228) > 2)
        options[count++] = 1;
    options[count++] = 0;
    if (at<Dword>(system(72), 176) == 1)
        options[count++] = 4;
    options[count++] = 6;
    options[count++] = 7;
    return next_option(current, options, count);
}

void __fastcall initialize_race_options(void *race, void *) {
    for (unsigned i = 0; i < 10; ++i)
        at<Dword>(race, 4 + i * 4) = 9;
    at<Dword>(race, 4) = 0;
    at<Dword>(race, 8) = 3;
    at<Dword>(race, 12) = 6;
    at<Dword>(race, 16) = 7;
}

void __fastcall initialize_elimination_options(void *race, void *) {
    for (unsigned i = 0; i < 10; ++i)
        at<Dword>(race, 4 + i * 4) = 9;
    at<Dword>(race, 4) = 8;
    at<Dword>(race, 8) = 0;
    at<Dword>(race, 12) = 2;
    at<Dword>(race, 16) = 6;
    at<Dword>(race, 20) = 7;
}

Dword __fastcall elimination_count(void *race, void *) {
    return at<Dword>(race, 34636);
}

void __fastcall reset_elimination_race(void *race, void *) {
    reset_race_statistics(race, nullptr);
    for (unsigned offset = 34628; offset <= 34640; offset += 4)
        at<Dword>(race, offset) = 0;
}

void __fastcall reset_time_trial(void *race, void *) {
    reset_race_statistics(race, nullptr);
    at<Dword>(race, 34628) = 0;
}

Dword __fastcall time_trial_elapsed(void *race, void *) {
    const Dword start = at<Dword>(race, 34628);
    if (static_cast<int>(start) < 1)
        return 0;
    return game::race_ticks - start;
}

Byte __fastcall elimination_last_position(void *race, void *, Dword slot, int checkpoint) {
    RaceParticipant *record = participant(race, slot);
    const Dword remaining =
        at<Dword>(system(72), 180) + at<Dword>(system(72), 176) - Dword(checkpoint);
    if (checkpoint < 0)
        return remaining == 1;
    return Dword(record->checkpoint_positions[checkpoint]) == remaining - 2;
}

Byte __fastcall elimination_complete(void *race, void *) {
    if (network::session_active()) {
        Dword retired = 0;
        extern int count_elimination_candidates(void *, Dword *);
        return count_elimination_candidates(race, &retired) < 2;
    }
    if (!invoke<Byte>(race, 25, Byte(0)))
        return 1;
    if (at<int>(system(72), 180) <= at<int>(race, 34628))
        return !invoke<Byte>(race, 25, Byte(0));
    return 0;
}

int __fastcall elimination_racer_count(void *race, void *) {
    if (network::session_active()) {
        Dword retired = 0;
        extern int count_elimination_candidates(void *, Dword *);
        return count_elimination_candidates(race, &retired);
    }
    return static_cast<int>(at<Dword>(system(72), 180) - at<Dword>(race, 34628) +
                            at<Dword>(system(72), 176));
}

Byte __fastcall timed_event_complete(void *race, void *) {
    if (event_complete(race, nullptr))
        return 1;
    const int limit = at<int>(system(72), 164);
    return limit < invoke<int>(race, 22);
}

int __fastcall timed_participant_remaining(void *race, void *, Dword slot) {
    alignas(4) Byte settings[72], rewards[12];
    read_race_reward_rules(system(72), settings, rewards);
    const Dword limit = at<Dword>(settings, 24) * 60;
    const int finished = participant(race, slot)->finish_time;
    if (finished >= 0)
        return static_cast<int>(limit - Dword(finished));
    if (invoke<int>(race, 22))
        return static_cast<int>(limit - Dword(invoke<int>(race, 22)));
    return static_cast<int>(limit);
}

void __fastcall begin_race_statistics(void *race, void *) {
    void *rules = system(72);
    if (at<Dword>(rules, 152) == 2 && at<Byte>(race, 44) && !at<Dword>(rules, 140))
        mark_event_entered(active_event_progress(rules));
    at<Dword>(race, 34600) = 0;
    invoke(race, 17);
}

void __fastcall begin_race_run(void *race, void *) {
    at<Dword>(race, 34612) = 0;
    at<Dword>(race, 34608) = 1;
    invoke(race, 18);
}

int __fastcall race_next_option(void *, void *, int current) {
    int options[8];
    int count = 0;
    if (at<int>(system(72), 228) > 1)
        options[count++] = 1;
    options[count++] = 0;
    if (at<Dword>(system(72), 176) == 1)
        options[count++] = 4;
    options[count++] = 6;
    options[count++] = 7;
    return next_option(current, options, count);
}

int count_elimination_candidates(void *race, Dword *local_count) {
    void *rules = system(72);
    int count = 0;
    Dword locals = 0;
    for (int index = 0; index < at<int>(rules, 168); ++index) {
        const Dword slot = Dword(index);
        if (participant_is_cop(race, slot) || participant_blocked(race, slot) ||
            participant_disconnected(race, slot))
            continue;
        alignas(4) RaceParticipant copy;
        network::world::copy_race_player(&copy, participant(race, slot));
        try {
            if (!copy.retired && !participant_has_finished(&copy) && copy.retirement_tick < 0) {
                ++count;
                void *entry = slot < 24 ? bytes(rules) + 248 + slot * 84 : nullptr;
                if (game::runtime::ai::lifecycle::pending::participant_enabled(race, slot) &&
                    !at<Dword>(entry, 4))
                    ++locals;
            }
        } catch (...) {
            network::world::destroy_race_player(&copy);
            throw;
        }
        network::world::destroy_race_player(&copy);
    }
    if (local_count)
        *local_count = locals;
    return count;
}
} // namespace nfs::application::pending
